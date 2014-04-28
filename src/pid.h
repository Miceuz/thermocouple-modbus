#ifndef __ADC_H
#include <inttypes.h>

/**
 * Initialises PID controlled with specified gains
 **/
void pidInit(uint8_t p, uint8_t i, uint8_t d);

/**
 * Sets the desired set point for controlled process
 **/
void pidSetSetpoint(unsigned long sp);

/**
 * Runs one PID iteration
 * @param processValue - a controlled process variable normed to [0..10000]
 * @return controll output normed to [0..255]
 **/
uint8_t pidRunIteration(unsigned long processValue);

#endif
