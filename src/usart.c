#include <avr/io.h>
#include "usart.h"

inline void usartReaderDisable() {
	READER_DISABLE_PORT |= _BV(READER_DISABLE_PIN);
}

inline void usartInit( unsigned int ubrr ){
	READER_DISABLE_DIR |= _BV(READER_DISABLE_PIN);//~RE pin of RS-485 transceiver

	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0);     //Enable TX
	UCSR0C = (3 << UCSZ00);    //asynchronous 8 N 1
}

void usartPuts( char *data ) {
	while ((*data != '\0'))
	   {
	      while (!(UCSR0A & (1 <<UDRE0)));
	      UDR0 = *data;
	      data++;
	   }
}

void usartWaitToFinish() {
	while (!(UCSR0A & _BV(UDRE0))); 	//data buffer is empty
	while (!(UCSR0A & _BV(TXC0)));		//TX shift register is empty
	UCSR0A |= _BV(TXC0); 				//clear the "TX shift register empy" flag
}
