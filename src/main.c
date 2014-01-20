#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "thermocouple_K.h"

//#define F_CPU 1000000UL
#define UBRR_1200 51
#define UBRR_2400 25     // for 1Mhz

// #define UBRR_2400 207 // for 8Mhz with .2% error
#define UBRR_19200 51  // for 8Mhz with .2% error
#define UBRR_38400 25 // for 8Mhz with .2% error
#define UBRR_115200 8 // for 8Mhz with .2% error

void dacWrite(unsigned int dacValue);
unsigned int currDacValue = 0;

 
inline void USART_init( unsigned int ubrr ){
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0);     // Enable RX, TX & RX interrupt
	UCSR0C = (3 << UCSZ00);    //asynchronous 8 N 1
}

/* Send some data to the serial port */

void USART_tx_string( char *data ) {
	while ((*data != '\0'))
	   {
	      while (!(UCSR0A & (1 <<UDRE0)));
	      UDR0 = *data;
	      data++;
	   }   
}

unsigned int tcFiltered = 0;

inline uint16_t filter(int32_t thisTc) {
	if(0 != tcFiltered) {
		tcFiltered = tcFiltered	+ ((thisTc - tcFiltered) >> 2);
	} else {
		tcFiltered = thisTc;
	}
	return (uint16_t) tcFiltered;
}

volatile unsigned long adcOversampled = 0;
volatile unsigned long oversamplingSum = 0;
volatile unsigned int sampleIndex = 0;
#define OVERSAMPLE_SAMPLES 256
#define OVERSAMPLE_SHIFT 4
volatile unsigned char oversamplingInProgress = 0;

ISR(ADC_vect) {
	if(!oversamplingInProgress) {
		return; //in case of ambient temperature conversion, just return	
	}
	TCNT0 = 0;
	oversamplingSum += (long)ADC;
	sampleIndex ++;

//	int noise = (abs(sampleIndex % 18 - 8)-4) * 10;
//	if(((long) currDacValue + (long) noise) > 0 && ((long) currDacValue + (long) noise) < 65536) {
//		dacWrite((long)currDacValue + noise);
//	}

	if(sampleIndex >= OVERSAMPLE_SAMPLES) {
		adcOversampled = oversamplingSum >> OVERSAMPLE_SHIFT;
		filter(adcOversampled);
		oversamplingSum = 0;
		sampleIndex = 0;
		oversamplingInProgress = 0;
	}
	TIFR0 |= _BV(OCF0A);//clear timer interrupt flag so that next ADC conversion would be triggered. as we don't service COMPA interrupt, flag is left set.
}

unsigned int readAdc(unsigned char channel) {
	while(_BV(ADSC) & ADCSRA) { //wait for possibly ongoing conversion to complete
	}
	ADCSRA &= ~_BV(ADATE); //disable autotriggering
	ADMUX = channel;
	ADCSRA |= _BV(ADSC);
	sleep_mode();
	return ADC;
}

unsigned long readAdcOversampled(unsigned char channel) {
	ADCSRB |= _BV(ADTS0) | _BV(ADTS1); //trigger ADC conversion on Timer0 Compare Match A
	ADCSRA |= _BV(ADATE); //ADC auto triggering enable

	OCR0A = 78; //compare match A @ 3.2kHz
	TCNT0 = 0;
	ADMUX = channel;
	oversamplingInProgress = 1;
	TCCR0B |= _BV(CS00) | _BV(CS01); //Fmcu/64
	while(oversamplingInProgress) {
		sleep_mode();
		//NOTHING, flag reset in ADC interrupt routine
	}
	TCCR0B = 0;
	return adcOversampled;
}

void debug() {
	char* buffer = "123456789";
	DDRC &= ~_BV(PC4);

//	int ambient = ((double)readAdc(0) * ((float)5000/(float)1024) - 1000)/20;
//	USART_tx_string(utoa(ambient, buffer, 10));
//	USART_tx_string(", ");
//	_delay_ms(1);

	while(!(PINC & _BV(PC4)));

	unsigned int adcValue = readAdcOversampled(1);
	USART_tx_string(utoa(adcValue, buffer, 10));
//	USART_tx_string(", ");

//	float amplifiedTc = adcOversampled * (float)5000 / (float)16384;
//	USART_tx_string(utoa(amplifiedTc * 10, buffer, 10));
//	USART_tx_string(", ");
//
//	double tcMv = (amplifiedTc - 97.79)/(float)101;
//	USART_tx_string(ltoa(tcMv*1000, buffer, 10));
//	USART_tx_string(", ");
//	_delay_ms(1);

//	long compensatedTemp = tc_K_uv_to_celsius(tcMv, ambient)*10;
//	USART_tx_string(itoa(compensatedTemp, buffer, 10));
	USART_tx_string("\r\n");
	waitUsartToFinish();
	_delay_us(500);
	while((PINC & _BV(PC4)));
}


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

void waitUsartToFinish() {
	while (!(UCSR0A & _BV(UDRE0)))
		;
	while (!(UCSR0A & _BV(TXC0)))
		;
	UCSR0A |= _BV(TXC0);
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

int main(void) {
	sei();
        set_sleep_mode(SLEEP_MODE_ADC);
	DDRB |= _BV(PB1) | _BV(PB2);
	USART_init(UBRR_115200);
	DIDR0 |= _BV(ADC0D) | _BV(ADC1D);
	ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
	ADCSRA |= _BV(ADIE); //ADC interrupt enable 

	USART_tx_string("Hello\r\n");

	while (1) {
//		PORTB |= _BV(PB1);
//		PORTB &= ~_BV(PB2);
//		_delay_ms(10);
		debug();
		//testAdcPerformance();
//		PORTB |= _BV(PB2);
//		PORTB &= ~_BV(PB1);
//		_delay_ms(10);
	}
}
