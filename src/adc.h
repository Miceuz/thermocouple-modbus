#ifndef __ADC_H
unsigned int readAdc(unsigned char channel);
unsigned long readAdcOversampled(unsigned char channel);
void adcInit();
#endif
