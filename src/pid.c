#include "pid.h"

static long integralTerm = 0;
static long previousProcessValue = 0;
static long setPoint = 0;

static uint8_t kP = 0;
static uint8_t kI = 0;
static uint8_t kD = 0;

void pidInit(uint8_t p, uint8_t i, uint8_t d) {
	kP = p;
	kI = i;
	kD = d;
}

void pidSetSetpoint(unsigned long sp) {
	setPoint = sp;
}

uint8_t pidRunIteration(unsigned long processValue) {

	long error = setPoint - processValue;  
	integralTerm += kI * error;

	if(integralTerm / 1000L > 10000L) {
		integralTerm = 1000L * 10000L;
	} 

	long dProcessValue = (processValue - previousProcessValue);
	previousProcessValue = processValue;

	//~ usartPuts(ltoa(kP * error, buffer, 10));
	//~ usartPuts(", ");
	//~ 
	//~ usartPuts(ltoa(integralTerm/1000L, buffer, 10));
	//~ usartPuts(", ");
	
	long controlOutput = kP * error + integralTerm/1000L - kD * dProcessValue;

	if(controlOutput > 10000L) { 
		controlOutput = 10000L; 
	}
	if(controlOutput < 0) { 
		controlOutput = 0; 
	}
	//~ usartPuts(ltoa(ret, buffer, 10));
	//~ usartPuts(", ");
	
	return controlOutput * 255L / 10000L; //norm controll output to [0..255]
}

