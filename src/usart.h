#ifndef __ADC_H

#define UBRR_1200 51
#define UBRR_2400 25     // for 1Mhz
// #define UBRR_2400 207 // for 8Mhz with .2% error
#define UBRR_19200 51  // for 8Mhz with .2% error
#define UBRR_38400 25 // for 8Mhz with .2% error
#define UBRR_115200 8 // for 8Mhz with .2% error

void usartWaitToFinish();
void usartPuts( char *data );
inline void usartInit( unsigned int ubrr );

#endif
