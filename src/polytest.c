#include "thermocouple_K.h"
#include <stdio.h>
void main(int argc, char** argv) {
	double voltage = 45.119;
	double ambientTemp = 20;
	printf("%f", tc_K_uv_to_celsius(voltage, ambientTemp));
}
