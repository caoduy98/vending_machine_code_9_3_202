#ifndef TEMPERATE_H
#define TEMPERATE_H


#include "pin_num.h"
#include <stdint.h>
#include <stdbool.h>

#define FX0         PE15         /* DHT21*/
#define DO1         PB2         /* Control heater */


void Humidity_Init();
bool Read_Humidity(float * pfValue);
void Control_Humidity(float humidity,uint8_t humdset);

#endif