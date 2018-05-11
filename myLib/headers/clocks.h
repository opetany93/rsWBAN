#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <stdint.h>
#include <stdbool.h>

int8_t startLFCLK(void);
bool isLFCLKstable(void);

int8_t startHFCLK(void);
void stopHFCLK(void);
bool isHFCLKstable(void);
bool isHFCLKstopped(void);

#endif
