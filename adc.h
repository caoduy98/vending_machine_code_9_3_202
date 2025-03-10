#ifndef ADC_H
#define ADC_H

#include "fsl_adc12.h"
#include "pin.h"


//#define ADC_TEMP ADC0
//#define CHANEL_GROUP    0U
//#define CHANEL_NUMBER_1 1U
//#define CHANEL_NUMBER_2 3U
//#define CHANEL_NUMBER_3 4U


// ADC0
#define CLOCK_ADC0              kCLOCK_Adc0
#define CLOCK_SOURCE_ADC0       kADC12_ClockSourceAlt0
#define ADC0_IRQ_HANDLER_FUNC   ADC0_IRQHandler

// ADC1
#define CLOCK_ADC1              kCLOCK_Adc1
#define CLOCK_SOURCE_ADC1       kADC12_ClockSourceAlt0
#define ADC1_IRQ_HANDLER_FUNC   ADC1_IRQHandler

/* Group ADC0*/
#define ADC0_CHANEL_GROUP     0U
// Chanel 0 is PA0 (Motor)
#define ADC0_CHANEL_MOTOR     0U
// Chanel 3 is PA7 (Battery)
#define ADC0_CHANEL_BATTERY   3U

/* Group ADC1*/
#define ADC1_CHANEL_GROUP     0U
// Chanel 0 is PA2 (Pin)
#define ADC1_CHANEL_PIN       0U
// Chanel 1 is PA3 (NTC+)
#define ADC1_CHANEL_NTC_P     1U
// Chanel 2 is PD2 (NTC-)
#define ADC1_CHANEL_NTC_N     2U

/********/
// ADC0 -- MOTOR
#define PIN_ADC_MOTOR     PA0      // adc of Motor

// ADC0 -- BATTERY
#define PIN_ADC_BATTERY   PA7      // adc of Battery

// ADC1 -- NTC
#define PIN_ADC_NTC_P     PA3      // adc of NTC+
#define PIN_ADC_NTC_N     PD2      // adc of NTC-

// ADC1 -- Pin
#define PIN_ADC_PIN       PA2      //  adc of Pin


void ADC0_Init();
void ADC1_Init();
uint32_t getAdc0(uint8_t chanel);
uint32_t getAdc1(uint8_t chanel);

#endif