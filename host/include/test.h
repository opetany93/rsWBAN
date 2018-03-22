#ifndef TEST_H_
#define TEST_H_

#include <stdint.h>

extern const uint8_t Apple [];

//========================== UART functions =========================
void echoOnOffUart(void);
void temperatureCoreUart(void);
void adxl345_init_test(void);
void LSM303DLH_acc_axes_uart_test(void);
void LSM303DLH_mag_axes_uart_test(void);

void lcd_Nokia_5110_SpinningLine(void);

void pwmInit(void);
void pulse_diodes(void);

#endif /* __TEST__H__ */
