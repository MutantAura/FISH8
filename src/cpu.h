#include "fish.h"

#ifndef CPU_H
#define CPU_H

// Used to store keypad state for instructions that need a point of comparison such as Fx0A.
uint8_t keypad_buffer[16];

void EmulateCpu(Fish* device);

#endif // CPU_H