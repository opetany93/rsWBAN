#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <stdint.h>

int8_t startLFCLK(void);
uint8_t isLFCLKstable(void);

int8_t startHFCLK(void);
void stopHFCLK(void);
uint8_t isHFCLKstable(void);
uint8_t isHFCLKstopped(void);

#endif
