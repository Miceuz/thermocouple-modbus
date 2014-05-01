#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "buffer.h"

#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0
#define UART0_RECEIVE_INTERRUPT USART_RX_vect
#define UART0_TRANSMIT_INTERRUPT USART_UDRE_vect

static volatile RingBuffer rxBuffer;
static volatile RingBuffer txBuffer;
static volatile uint8_t UART_LastRxError;

static volatile uint8_t rxInProgress = 0;

static inline void usartReaderDisable() {
	READER_DISABLE_PORT |= _BV(READER_DISABLE_PIN);
}

inline void usartInit( unsigned int ubrr ){
	READER_DISABLE_DIR |= _BV(READER_DISABLE_PIN);//~RE pin of RS-485 transceiver
	usartReaderDisable();

	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	UCSR0B = (1 << TXEN0);     //Enable TX
	UCSR0C = (3 << UCSZ00);    //asynchronous 8 N 1

	ringBufferInit(&txBuffer);
	ringBufferInit(&rxBuffer);
}

/*************************************************************************
Function: UART Receive Complete interrupt
Purpose: called when the UART has received a character
**************************************************************************/
ISR(UART0_RECEIVE_INTERRUPT) {
	ringBufferWrite(&rxBuffer, UART0_DATA);
	UART_LastRxError = (UART0_STATUS & (_BV(FE0)|_BV(DOR0)) );
}

/*************************************************************************
Function: UART Data Register Empty interrupt
Purpose: called when the UART is ready to transmit the next byte
**************************************************************************/
ISR(UART0_TRANSMIT_INTERRUPT) {
	if(!ringBufferIsEmpty(&txBuffer)) {
		PORTB |= _BV(PB1);
		uint8_t data = 0;
		if(RING_BUFFER_STATUS_OK == ringBufferRead(&txBuffer, &data)) {
			UART0_DATA = data;
		} else {
//			ringBufferClear(&txBuffer);
			UART0_CONTROL &= ~_BV(UART0_UDRIE);
			PORTB &= ~_BV(PB1);
		}
	} else {
		/* tx buffer empty, disable UDRE interrupt */
		UART0_CONTROL &= ~_BV(UART0_UDRIE);
		PORTB &= ~_BV(PB1);
	}
}

void uartPutc(unsigned char data) {
	if(rxInProgress) {
		ringBufferWrite(&txBuffer, data);
		return;
	} else {
		while (ringBufferIsFull(&txBuffer)){
			;//wait for free space in buffer
			UART0_CONTROL |= _BV(UART0_UDRIE);
			PORTB |= _BV(PB2);
		} 
		PORTB &= ~_BV(PB2);
		ringBufferWrite(&txBuffer, data);
	}
	/* enable UDRE interrupt */
	UART0_CONTROL |= _BV(UART0_UDRIE);
}

void usartPuts(char *data) {
	while ((*data != '\0')) {
		uartPutc(*data);
		data++;
	}
}

void usartPutsSynchroneous( char *data ) {
	while ((*data != '\0')) {
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
