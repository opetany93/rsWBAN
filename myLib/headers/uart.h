#ifndef UART_H_
#define UART_H_

#define NRF_UART_PSEL_DISCONNECTED 0xFFFFFFFF

#include <stdint.h>

//===============================================================================================
void uartInit(void);
void uartWrite(char data);
void uartWriteS(char *s);
void setUartIrqFunc(void (*pFunc)(char data));

void uartDmaInit(void);
void uartDmaWriteS(char *s, uint32_t length);

//===============================================================================================

#endif
