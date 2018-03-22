#include "i2c.h"
#include "hal.h"

#include "mydefinitions.h"

//===============================================================================================
/*
przekazywane sa 4 argumenty:
	modul: TWI0 albo TWI1
	
	speed:	FREQUENCY_K100 - 100 kHz
			FREQUENCY_K250 - 250 kHz
			FREQUENCY_K400 - 400 kHz
	
	pin_SCL:	numer pinu SCL
	pin_SDA:	numer pinu SDA
*/
void i2cInit(NRF_TWI_Type* TWIx, uint32_t speed, uint8_t pin_SCL, uint8_t pin_SDA)
{
	//gpio config
	GPIO->PIN_CNF[pin_SCL] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
							| (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
							| (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos));

	GPIO->PIN_CNF[pin_SDA] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
							| (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)
							| (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos));
	
	TWIx->INTENSET = ((TWI_INTENSET_RXDREADY_Enabled << TWI_INTENSET_RXDREADY_Pos)
					 |(TWI_INTENSET_TXDSENT_Enabled  << TWI_INTENSET_TXDSENT_Pos));
	
	//pins select
	TWIx->PSELSCL = pin_SCL;
	TWIx->PSELSDA = pin_SDA;
	
	//frequency set
	TWIx->FREQUENCY = speed;
		
	//i2c enable
	TWI_ENABLE(TWIx);
}


//=======================================================================================
void i2cWrite(NRF_TWI_Type* TWIx, uint8_t device_address, uint8_t register_address, uint8_t data)
{
	TWIx->ADDRESS = device_address;				//adres urzadzenia slave
	TWIx->TXD = register_address;				//wpisz do rejestru TXD wartosc hex rejestru slave
	TWIx->TASKS_STARTTX = 1U;
	
	while(!(TWIx->EVENTS_TXDSENT));				//czekaj az wysle zawartosc rejestru TXD
	TWIx->EVENTS_TXDSENT = 0U;					//czysc flage
	
	TWIx->TXD = data;
	
	while(!(TWIx->EVENTS_TXDSENT));				//czekaj az wysle zawartosc rejestru TXD
	TWIx->EVENTS_TXDSENT = 0U;					//czysc flage
	
	TWIx->TASKS_STOP = 1U;
}


//=======================================================================================
uint8_t i2cRead(NRF_TWI_Type* TWIx, uint8_t device_address, uint8_t register_address)
{
	uint8_t buf;
	
	TWIx->ADDRESS = device_address;				//adres urzadzenia slave
	TWIx->TXD = register_address;				//wpisz do rejestru TXD wartosc hex rejestru slave
	TWIx->TASKS_STARTTX = 1U;					// start transmisji write to slave
	
	while(!(TWIx->EVENTS_TXDSENT));				//czekaj az wysle zawartosc rejestru TXD
	TWIx->EVENTS_TXDSENT = 0U;					//czysc flage
	
	TWIx->TASKS_STARTRX = 1U;
	
	while(!(TWIx->EVENTS_RXDREADY));			//czekaj az dane beda gotowe do odczytu
	TWIx->EVENTS_RXDREADY = 0U;					//czysc flage
	
	buf = TWIx->RXD;
	
	TWIx->TASKS_STOP = 1U;						//wyslij znak stopu
	
	return buf;
}


//=================================================================================================
/*odczyt typu multibyte nastepuje poprzez zapis odebranych bajtow za pomoca podania 
	wskaznika buf w strukturze/tablicy na ktora pokazuje wskaznik

	parametr length okresla ile odczytanych ma byc bajtow

*/
void i2cReadMultibyte(NRF_TWI_Type* TWIx, uint8_t device_address, uint8_t register_address, uint8_t* buf, uint8_t length)
{
	uint8_t i;
	
	buf += (length - 1);
	
	TWIx->ADDRESS = device_address;							//adres urzadzenia slave
	TWIx->TXD = register_address | READ_MULTIBYTE_CMD;		//wpisz do rejestru TXD wartosc hex rejestru slave device
	TWIx->TASKS_STARTTX = 1U;								// start transmisji write to slave device
	
	while(!(TWIx->EVENTS_TXDSENT));							// czekaj az wysle zawartosc rejestru TXD
	TWIx->EVENTS_TXDSENT = 0;								// czysc flage
	
	TWIx->TASKS_STARTRX = 1U;
	
	for (i = 0; i < length; i++)
	{
		while(!(TWIx->EVENTS_RXDREADY));					// czekaj az dane beda gotowe do odczytu
		TWIx->EVENTS_RXDREADY = 0U;							// czysc flage
	
		*buf = TWIx->RXD;									// odczytaj dane
		 buf--;
	}
	
	TWIx->TASKS_STOP = 1U;									// wyslij znak stopu
}
