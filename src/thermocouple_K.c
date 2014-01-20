/*
  Copyright (c) 2012, ';DROP TABLE teams;, University of Adelaide.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met: 

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution. 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those
  of the authors and should not be interpreted as representing official policies, 
  either expressed or implied, of the University of Adelaide.
  */  

/**
 * Type K thermocouple library.
 * Converts uV readings from an ADC to a temperature,
 * over a range from 0 - 1372 C.<br/>
 * Requires a cold junction reference temperature. <br/>
 * Based on information found in National Instruments 
 * Application Note 043: Measuring Temperature with
 * Thermocouples - a Tutorial and ITS-90 Thermocouple
 *Direct and Inverse Polynomials.
 */

#include <math.h>

#include "thermocouple_K.h"
#define LEN(x) (sizeof(x)/sizeof(x[0]))
#define SQR(x) ((x)*(x))
#define A0 ( 0.118597600000E+00)
#define A1 (-0.118343200000E-03)
#define A2 ( 0.126968600000E+03)

/**
 * Polynomial coefficients for type K
 * thermocouples (mV to T).
 */
float tc_K_VT_coefficients[10] = { 0.0,
                                   2.508355E01,
                                   7.860106E-02,
                                   -2.503131E-01,
                                   8.315270E-02,
                                   -1.228034E-02,
                                   9.804036E-04,
                                   -4.413030E-05,
                                   1.057734E-06,
                                   -1.052755E-08 };

float tc_K_VT_coefficients_500[7] = { -1.318058E02,
                                   4.830222E01,
                                   -1.646031E00,
                                   5.464731E-02,
                                   -9.650715E-04,
                                   8.802193E-06,
                                   -3.110810E-08};

/**
 * Polynomial coefficients for type K
 * thermocouples (T to mV).
 */
float tc_K_TV_coefficients[10] = {  -0.17600413686E-01,
                                    0.38921204975E-01,
                                    0.185587700E-04,
                                    -0.99457593E-07,
                                    0.318409457E-09,
                                    -0.5607824E-12,
                                    0.56075059E-15,
                                    -0.3202072E-18,
                                    0.97151147E-22,
                                    -0.1210472E-25 };

static double poly(double x, const float *coefs, int n){
   double y=0., p=1.;
   int i;

   for (i=0; i<n; i++){
      y += coefs[i]*p;
      p *= x;
   }

   return y;
}

/**
 * Calculate the hot junction temperature,
 * given a mV reading and cold junction temperature.
 * @param voltage   The voltage reading in uV.
 * @param cj_temp   The cold junction temperature.
 * @return          The compensated hot junction temperature.
 */
float tc_K_uv_to_celsius(float voltage, float cj_temp)
{   
    /* Convert cold junction temp to equivalent K-type uV */
    double cj_temp_uv = tc_K_celsius_to_uv(cj_temp);
    /* Add to measured voltage */
    double compensated_uv = voltage + cj_temp_uv;
    /* ITS-90 polynomial T = a0 + a1*v + a2*v^2 + ... + an*v^n */
    double result;
    if(compensated_uv < 20.644) {
	    result = poly(compensated_uv, tc_K_VT_coefficients, 10);
    } else {
	    result = poly(compensated_uv, tc_K_VT_coefficients_500, 7);
    }    

    return result;
}

/**
 * Convert degrees Celsius to microvolts for type K.
 * @param   temp    The temperature in celsius
 * @return  voltage The voltage in uV.
 */
double tc_K_celsius_to_uv(float temp)
{
    // ITS-90 polynomial v = c0 + c1*T + c2*T^2 + ... + cn*T^n
    double result = poly(temp, tc_K_TV_coefficients, 10);
    result = result  + A0 * exp(A1 * SQR(temp - A2));;
    return result;
}
