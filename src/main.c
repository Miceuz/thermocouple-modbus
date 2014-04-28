#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "usart.h"
#include "thermocouple.h"
#include "I2CmasterTWI.h"

#define INSTAMP_GAIN 101
#define MAX_PROCESS_VALUE 170000L

unsigned int currDacValue = 0;
unsigned int tcFiltered = 0;
char* buffer = "123456789ABCDEF00";

void dacWrite(unsigned int dacValue);

#define CS PD5
#define SCK PD6
#define DATA PB0

void spiTick() {
	PORTD |= _BV(SCK);
	PORTD &= ~_BV(SCK);
}

void dacWrite(unsigned int dacValue) {
	PORTD &= ~_BV(CS);//assert CS

	PORTB &= ~_BV(DATA);
	spiTick();
	spiTick();

	char bitToSend = 15;
	while(bitToSend-- >= 0) {
		if((dacValue & 0x8000) > 0) {
			PORTB |= _BV(DATA);
		} else {
			PORTB &= ~_BV(DATA);
		}
		dacValue = dacValue << 1;
		spiTick();
	}

  PORTD |= _BV(CS);//deassert CS
}


void testAdcPerformance() {
	DDRD |= _BV(CS); //CS
	DDRD |= _BV(SCK); //SCK
	DDRB |= _BV(DATA); //DATA

	PORTD |= _BV(CS); //CS LOW

	char* buffer = "123456789";
	unsigned int i = 0;
	while(1) {
		currDacValue = i++;
		dacWrite(currDacValue);
		waitUsartToFinish();
		unsigned int adcValue = readAdcOversampled(6);
		USART_tx_string(utoa(i, buffer, 10));
		USART_tx_string(", ");
		USART_tx_string(utoa(adcValue, buffer, 10));
		USART_tx_string("\r\n");
	}
}

static inline uint16_t filter(int32_t thisTc) {
	if(0 != tcFiltered) {
		tcFiltered = tcFiltered	+ ((thisTc - tcFiltered) >> 2);
	} else {
		tcFiltered = thisTc;
	}
	return (uint16_t) tcFiltered;
}


static inline void debug() {

//	int ambient = ((double)readAdc(0) * ((float)5000/(float)1024) - 1000)/20; //cia buvo kazkoks grib
//  (x * 5 / 1023. - 1)/0.02 -- teisinga formule temperaturai paskaiciuoti
//  x * (5 / 0.02) / 1023 - 1/0.02
//  x * 250 / 1023 - 50
//  x * (250 * 1024/1023) / 1024 - 50

	long adc = (long)readAdc(0);
//	usartPuts(ltoa(adc, buffer, 10));
//	usartPuts(", ");
//	usartWaitToFinish();

	long ambient = ((adc * 250245L) >> 10) - 50000L; //units - 1/1000 degree C

//	usartPuts(ltoa(ambient, buffer, 10));
//	usartPuts(", ");
//	usartWaitToFinish();

	unsigned long adcValue = readAdcOversampled(1);
	filter(adcValue);

	//~ USART_tx_string(utoa(adcValue, buffer, 10));
	//~ USART_tx_string(", ");
	//~ waitUsartToFinish();

	unsigned long offset = readAdcOversampled(2);
	//~ USART_tx_string(utoa(offset, buffer, 10));
	//~ USART_tx_string(", ");
	//~ waitUsartToFinish();

//	unsigned long amplifiedTc = ((long)readAdcOversampled(1) - (long)offset) * (float)5000 / (float)16383 / INSTAMP_GAIN;
	unsigned long tcMicrovolts = (((long)readAdcOversampled(1) - (long)offset) * 49508) >> 14; //units - 1 uV

	long compensatedTemp = thermocoupleConvertWithCJCompensation(tcMicrovolts, ambient);
	usartPuts(ltoa(compensatedTemp, buffer, 10));
	usartPuts(", ");

	uint8_t controlSignal = pidRunIteration(compensatedTemp	* 10000L / MAX_PROCESS_VALUE);
	
	usartPuts(itoa(controlSignal, buffer, 10));
	usartPuts("\r\n");
	usartWaitToFinish();

	I2CTWI_transmit2Bytes(0x20 << 1, 1, controlSignal);
	I2CTWI_getState();
	_delay_ms(10);
}

void onI2CError(uint8_t requestId) {
	usartPuts("Transmission error\r\n");
}

#define READER_ENABLE PD2

static inline void readerEnable() {
	DDRD |= _BV(PD2);
	PORTD |= _BV(READER_ENABLE);
}

void main(void) {
	sei();
	adcInit();
	pidInit(16, 5, 0);
	pidSetSetpoint(5000);
	DDRB |= _BV(PB1) | _BV(PB2);
	usartInit(UBRR_115200);
	readerEnable();
	
	PORTC |= _BV(PC5) | _BV(PC4);//enable I2C weak pullups

	PORTC |= _BV(PC5) | _BV(PC4);//enable weak pullups

	usartPuts("Hello\r\n");
	usartWaitToFinish();
	usartPuts("how are you? \r\n");
	usartWaitToFinish();
	I2CTWI_initMaster(100);
	I2CTWI_setTransmissionErrorHandler(onI2CError);
	//I2CTWI_transmitByte(0x20 << 1, 12);
//	I2CTWI_getState();
	
	//~ I2CTWI_transmit2Bytes(0x20<<1, 1, 225);
	//~ I2CTWI_getState();

	while (1) {
//		PORTB |= _BV(PB1);
//		PORTB &= ~_BV(PB2);
//		_delay_ms(10);
		debug();
		task_I2CTWI();
		//~ unsigned long i;
		//~ char* buffer = "123456789ABCDEF00";
		//~ for(i = 0; i < 16383; i++) {
			//~ usartPuts(ltoa(i*100/16383, buffer, 10));
			//~ usartPuts(", ");
//~
			//~ unsigned long tcMv = ((long)i * 49504L) >> 14; //units - 1 uV
			//~ long compensatedTemp = mvToC(85, tcMv);
			//~ usartPuts(ltoa(compensatedTemp, buffer, 10));
			//~ usartPuts("\r\n");
			//~ usartWaitToFinish();
		//~ }
		//testAdcPerformance();
//		PORTB |= _BV(PB2);
//		PORTB &= ~_BV(PB1);
//		_delay_ms(10);
	}
}
