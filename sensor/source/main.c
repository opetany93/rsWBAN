#include "hal.h"
#include "radio.h"
#include "rtc.h"
#include "protocol.h"
#include "ADXL362.h"
#include "nrf.h"
#include "mydefinitions.h"
#include "spi.h"

#include <string.h>

#include "nrf_ppi.h"
#include "nrf_spim.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"
#include "nrf_rtc.h"

#define WRITE_CMD								0x0A
#define READ_CMD								0x0B

#define ONE_READ_BUFFER_SIZE 8

Rtc* rtcAdxl362 = NULL;

typedef struct ArrayList
{
	uint8_t buffer[ONE_READ_BUFFER_SIZE];

} ArrayList_type;

uint8_t txBuffer[2], initBuffer[3];
ArrayList_type adxl362ReadXYZSamplesBuffer[5];		//LSB first

uint8_t dataToSend[5][6];


static void configCsPin(void)
{
	nrf_gpio_cfg_output(CS_PIN);

	nrf_gpiote_task_configure(1, CS_PIN, NRF_GPIOTE_POLARITY_HITOLO, NRF_GPIOTE_INITIAL_VALUE_HIGH);
	nrf_gpiote_task_enable(1);
}

static void configPPIforSPIMandADXL362(void)
{
	nrf_ppi_channel_and_fork_endpoint_setup(NRF_PPI_CHANNEL2, (uint32_t) &RTC0->EVENTS_COMPARE[1], (uint32_t) &rtcAdxl362->RTCx->TASKS_STOP,(uint32_t) &RTC1->TASKS_CLEAR);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL2);

	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL3, (uint32_t) &NRF_SPIM1->EVENTS_END, (uint32_t) &GPIOTE->TASKS_SET[1]);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL3);

	nrf_ppi_channel_and_fork_endpoint_setup(NRF_PPI_CHANNEL4, (uint32_t) &rtcAdxl362->RTCx->EVENTS_COMPARE[0], (uint32_t) &NRF_SPIM1->TASKS_START, (uint32_t) &GPIOTE->TASKS_CLR[1]);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL4);

	nrf_ppi_channel_endpoint_setup(NRF_PPI_CHANNEL5, (uint32_t) &rtcAdxl362->RTCx->EVENTS_COMPARE[0], (uint32_t) &rtcAdxl362->RTCx->TASKS_CLEAR);
	nrf_ppi_channel_enable(NRF_PPI_CHANNEL5);
}

static void configSpim1(void)
{
	nrf_gpio_cfg_output(SCK_PIN);
	nrf_gpio_cfg_output(MOSI_PIN);
	nrf_gpio_cfg_input(MISO_PIN, NRF_GPIO_PIN_NOPULL);

	nrf_spim_pins_set(NRF_SPIM1, SCK_PIN, MOSI_PIN, MISO_PIN);
	nrf_spim_frequency_set(NRF_SPIM1, NRF_SPIM_FREQ_1M);
	nrf_spim_rx_buffer_set(NRF_SPIM1,(uint8_t *) &adxl362ReadXYZSamplesBuffer, 3);
	nrf_spim_rx_list_enable(NRF_SPIM1);
	nrf_spim_tx_buffer_set(NRF_SPIM1, initBuffer, 3);
	nrf_spim_configure(NRF_SPIM1, NRF_SPIM_MODE_0, NRF_SPIM_BIT_ORDER_MSB_FIRST);
	nrf_spim_orc_set(NRF_SPIM1, 0x00);
	nrf_spim_enable(NRF_SPIM1);
}

void rtcForAdxl362Config(void)
{
	rtcAdxl362 = rtcInit(RTC1);

	rtcAdxl362->setPrescaler(rtcAdxl362, 0);
	rtcAdxl362->setCCreg(rtcAdxl362, RTC_CHANNEL0, 295);
	rtcAdxl362->compareEventEnable(rtcAdxl362, RTC_CHANNEL0);
	rtcAdxl362->clear(rtcAdxl362);
}

static void initAdxl362()
{
	initBuffer[0] = WRITE_CMD;
	initBuffer[1] = ADXL362_POWER_CTL;
	initBuffer[2] = (1 << ADXL362_MEASURE);

	GPIOTE->TASKS_CLR[1] = 1;
	NRF_SPIM1->TASKS_START = 1U;
	while(!NRF_SPIM1->EVENTS_END);
	NRF_SPIM1->EVENTS_END = 0;

	initBuffer[0] = WRITE_CMD;
	initBuffer[1] = ADXL362_FILTER_CTL;
	initBuffer[2] = 0x07;		// set 400 kHz sampling

	GPIOTE->TASKS_CLR[1] = 1;
	NRF_SPIM1->TASKS_START = 1U;
	while(!NRF_SPIM1->EVENTS_END);
	NRF_SPIM1->EVENTS_END = 0;

	txBuffer[0] = READ_CMD;
	txBuffer[1] = ADXL362_POWER_CTL;
	nrf_spim_tx_buffer_set(NRF_SPIM1, txBuffer, 2);
	nrf_spim_rx_buffer_set(NRF_SPIM1,(uint8_t *) &adxl362ReadXYZSamplesBuffer, 3);

	GPIOTE->TASKS_CLR[1] = 1;
	NRF_SPIM1->TASKS_START = 1U;
	while(!NRF_SPIM1->EVENTS_END);
	NRF_SPIM1->EVENTS_END = 0;

	if ((1 << ADXL362_MEASURE) != adxl362ReadXYZSamplesBuffer[0].buffer[2])
	{
		error();
	}
}


//=======================================================================================
int main(void)
{
	boardInit();
	Radio* radio = radioSensorInit();
	Rtc* rtc0 = rtcInit(RTC0);
	buttonInterruptInit();

	// ========== init for SPIM1 and RTC1 and ADXL362 100kHz
	rtcForAdxl362Config();
	configCsPin();
	configPPIforSPIMandADXL362();
	configSpim1();
	// ================================

	initAdxl362();

	txBuffer[0] = READ_CMD;
	txBuffer[1] = ADXL362_XDATA_L;
	nrf_spim_tx_buffer_set(NRF_SPIM1, txBuffer, 2);
	nrf_spim_rx_buffer_set(NRF_SPIM1,(uint8_t *) &adxl362ReadXYZSamplesBuffer, ONE_READ_BUFFER_SIZE);

	initProtocol(radio,rtc0);

	while(1)
	{
		sleep();
	}
}

void syncCallback(void)
{
	for(uint8_t k = 0; k < 5; k++)
	{
		memcpy(&dataToSend[k], &adxl362ReadXYZSamplesBuffer[k].buffer[2], 6);
	}

	nrf_spim_rx_buffer_set(NRF_SPIM1,(uint8_t *) &adxl362ReadXYZSamplesBuffer, ONE_READ_BUFFER_SIZE);

	nrf_rtc_task_trigger(rtcAdxl362->RTCx, NRF_RTC_TASK_START);
}

void timeSlotCallback(data_packet_t* dataPacketPtr)
{
	memcpy(dataPacketPtr->data, dataToSend, 30);
}
