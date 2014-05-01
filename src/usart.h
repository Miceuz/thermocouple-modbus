#ifndef __ADC_H

#define READER_DISABLE_PIN PD2
#define READER_DISABLE_PORT PORTD
#define READER_DISABLE_DIR DDRD

#define UBRR_1200 51
#define UBRR_2400 25     // for 1Mhz
// #define UBRR_2400 207 // for 8Mhz with .2% error
#define UBRR_19200 51  // for 8Mhz with .2% error
#define UBRR_38400 25 // for 8Mhz with .2% error
#define UBRR_115200 8 // for 8Mhz with .2% error

void usartWaitToFinish();
void usartPuts( char *data );
void uartPutc(unsigned char data);
uint16_t usartGetc();

extern inline void usartInit( unsigned int ubrr );
#endif
