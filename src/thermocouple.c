#define POINTS_COUNT 65

typedef	struct {
	long temp;
	unsigned long millivolts;
} temp_point;

static temp_point thermocouplePoints[] = {
{ 0 , 0 },
{ 10000 , 397 },
{ 20000 , 798 },
{ 30000 , 1203 },
{ 40000 , 1612 },
{ 50000 , 2023 },
{ 60000 , 2436 },
{ 79000 , 3225 },
{ 98000 , 4013 },
{ 116000 , 4756 },
{ 134000 , 5491 },
{ 139000 , 5694 },
{ 155000 , 6339 },
{ 172000 , 7021 },
{ 193000 , 7859 },
{ 212000 , 8619 },
{ 231000 , 9383 },
{ 250000 , 10153 },
{ 269000 , 10930 },
{ 288000 , 11712 },
{ 307000 , 12499 },
{ 326000 , 13290 },
{ 345000 , 14084 },
{ 364000 , 14881 },
{ 383000 , 15680 },
{ 402000 , 16482 },
{ 421000 , 17285 },
{ 440000 , 18091 },
{ 459000 , 18898 },
{ 478000 , 19707 },
{ 497000 , 20516 },
{ 516000 , 21326 },
{ 535000 , 22137 },
{ 554000 , 22947 },
{ 573000 , 23757 },
{ 592000 , 24565 },
{ 611000 , 25373 },
{ 630000 , 26179 },
{ 649000 , 26983 },
{ 668000 , 27784 },
{ 687000 , 28584 },
{ 706000 , 29380 },
{ 725000 , 30174 },
{ 744000 , 30964 },
{ 763000 , 31752 },
{ 782000 , 32536 },
{ 801000 , 33316 },
{ 820000 , 34093 },
{ 839000 , 34867 },
{ 858000 , 35637 },
{ 877000 , 36403 },
{ 896000 , 37166 },
{ 915000 , 37925 },
{ 934000 , 38680 },
{ 953000 , 39432 },
{ 972000 , 40180 },
{ 991000 , 40924 },
{ 1010000 , 41665 },
{ 1029000 , 42402 },
{ 1048000 , 43134 },
{ 1067000 , 43863 },
{ 1086000 , 44588 },
{ 1105000 , 45308 },
{ 1124000 , 46024 },
{ 1143000 , 46735 },
{ 1200000 , 48838 }
};

static inline unsigned long interpolate(unsigned long val, unsigned long rangeStart, unsigned long rangeEnd, unsigned long valStart, unsigned long valEnd) {
    //~ printf ("(rangeEnd - rangeStart) = %lu - %lu = %lu\n",rangeEnd, rangeStart, (rangeEnd - rangeStart));
    //~ printf ("(val - valStart) = %lu - %lu = %lu\n", val, valStart, (val - valStart));
    //~ printf ("(rangeEnd - rangeStart) * (val - valStart) = %lu\n", (rangeEnd - rangeStart) * (val - valStart));
    //~ printf ("(valEnd - valStart) = %lu-%lu=%lu\n", valEnd, valStart, (valEnd - valStart));
    return rangeStart + (rangeEnd - rangeStart) * (val - valStart) / (valEnd - valStart);
}

static inline unsigned long interpolateVoltage(unsigned long temp, unsigned char i){
//	printf("interpolating voltage of %lu\n", temp);
    return interpolate(temp, thermocouplePoints[i-1].millivolts, thermocouplePoints[i].millivolts, thermocouplePoints[i-1].temp, thermocouplePoints[i].temp);
}

static inline unsigned long interpolateTemperature(unsigned long millivolts, unsigned char i){
//	printf("interpolating temp of %lu at %d \n", millivolts, i);
    return interpolate(millivolts, thermocouplePoints[i-1].temp, thermocouplePoints[i].temp, thermocouplePoints[i-1].millivolts, thermocouplePoints[i].millivolts);
}

static unsigned long tempGetter(unsigned char i) {
	return thermocouplePoints[i].temp;
}

static unsigned long millivoltsGetter(unsigned char i) {
	return thermocouplePoints[i].millivolts;
}

static inline unsigned char search(unsigned long value, unsigned long (* getter)(unsigned char)) {
	unsigned char i;
	for(i = 0; i < POINTS_COUNT; i++) {
		if(getter(i) > value) {
			return i;
		}
	}
	return POINTS_COUNT-1;
}

static inline unsigned char searchTemp(unsigned long temp) {
	unsigned char i;
	for(i = 0; i < POINTS_COUNT; i++) {
		if(thermocouplePoints[i].temp > temp) {
			return i;
		}
	}
	return POINTS_COUNT-1;
}

static inline unsigned char searchMillivolts(unsigned long millivolts) {
	unsigned char i;
	for(i = 0; i < POINTS_COUNT; i++) {
		if(thermocouplePoints[i].millivolts > millivolts) {
			return i;
		}
	}
	return POINTS_COUNT-1;
}

#include <stdio.h>

//TODO ambientine temperatura gali buti < 0 !!!

long mvToC(unsigned long ambient, unsigned long millivolts) {
//	printf("%ld\n", interpolateVoltage(ambient, searchTemp(ambient)));
	millivolts += interpolateVoltage(ambient, searchTemp(ambient));
//	printf("%ld\n", millivolts);
	return interpolateTemperature(millivolts, searchMillivolts(millivolts));
}

//~ void main(int argc, char **argv) {
	//~ printf("%ld\n", mvToC(80000, 45119));
//~ }

