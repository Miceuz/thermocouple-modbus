#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define OVERSAMPLE_SAMPLES 256
#define OVERSAMPLE_SHIFT 4

volatile unsigned long adcOversampled = 0;
volatile unsigned long oversamplingSum = 0;
volatile unsigned int sampleIndex = 0;
volatile unsigned char oversamplingInProgress = 0;


ISR(ADC_vect) {
	if(!oversamplingInProgress) {
		return; //in case of non oversampling conversion, just return
	}
	oversamplingSum += (long)ADC;
	sampleIndex ++;

	if(sampleIndex >= OVERSAMPLE_SAMPLES) {
		TCCR0B = 0; //stop sampling timer
		oversamplingInProgress = 0;
	}
	TIFR0 |= _BV(OCF0A);//clear timer interrupt flag so that next ADC conversion would be triggered.
						//as we don't service COMPA interrupt, we have to clean the flag manually.
}

unsigned int readAdc(unsigned char channel) {
	ADCSRA &= ~_BV(ADATE); 		//disable autotriggering
	while(_BV(ADSC) & ADCSRA);	//wait for possibly ongoing conversion to complete

	ADMUX = channel;

	ADCSRA |= _BV(ADSC);		//start conversion
	while(ADCSRA & _BV(ADSC)) {
		//NOTHING, the first conversion after MUX change is dirty
	}

	ADCSRA |= _BV(ADSC);		//start conversion
	while(ADCSRA & _BV(ADSC)) {
		sleep_mode();//sleep during second conversion to get better results
	}

	return ADC;
}

unsigned long readAdcOversampled(unsigned char channel) {
	oversamplingSum = 0;
	sampleIndex = 0;
	oversamplingInProgress = 1;

	ADCSRB |= _BV(ADTS0) | _BV(ADTS1); 		//trigger ADC conversion on Timer0 Compare Match A
	ADCSRA |= _BV(ADATE); 					//ADC auto triggering enable

	ADMUX = channel;

	TCCR0B |= _BV(CS00) | _BV(CS01); 		//start sampling timer @ Fmcu/64
	
	while(oversamplingInProgress) {
		//NOTHING, timer interrupt flag is reset in ADC interrupt routine
	}

	adcOversampled = oversamplingSum >> OVERSAMPLE_SHIFT;

	return adcOversampled;
}


inline void adcInit() {
    set_sleep_mode(SLEEP_MODE_ADC);
	DIDR0 |= _BV(ADC0D) | _BV(ADC1D);			//disable digital input buffer on ADC pins
	ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); //enable ADC on the slowest clock - 125kHz
	ADCSRA |= _BV(ADIE); 						//ADC interrupt enable

	TCCR0A = _BV(WGM01);// |_BV(COM0A0)     // clear timer on compare match  
	OCR0A = 78; 							//compare match A @ 3.2kHz - this will be the sampling frequency
	
	DDRD |= _BV(PD3); //OC2B pin as output
	OCR2A = 78;
	TCCR2A = _BV(COM2B0) | _BV(WGM21);
	TCCR2B |= _BV(CS20) | _BV(CS21) | _BV(CS22);
}
