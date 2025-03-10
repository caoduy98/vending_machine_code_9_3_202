#include "adc.h"
#include "dbg_console.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_ADC
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif

volatile bool adc0_flag = false;
volatile bool adc1_flag = false;

uint32_t adc0ChanelValue = 0;
uint32_t adc1ChanelValue = 0;

adc12_config_t          adc0ConfigStruct;
adc12_channel_config_t  adc0Channel0ConfigStruct;
adc12_channel_config_t  adc0Channel3ConfigStruct;

adc12_config_t          adc1ConfigStruct;
adc12_channel_config_t  adc1Channel0ConfigStruct;
adc12_channel_config_t  adc1Channel1ConfigStruct;
adc12_channel_config_t  adc1Channel2ConfigStruct;

/**
 * @brief   Ham xu ly ngat ADC0
 *
 * @param   NONE
 * @retval  adc0ChanelValue -> Gia tri uint32_t cua ADC0
 */
void ADC0_IRQ_HANDLER_FUNC(void)
{
    adc0_flag = true;
    /* Read conversion result to clear the conversion completed flag. */
    adc0ChanelValue = ADC12_GetChannelConversionValue( ADC0, ADC0_CHANEL_GROUP);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}
/**
 * @brief   Ham xu ly ngat ADC1
 *
 * @param   NONE
 * @retval  adc1ChanelValue -> Gia tri uint32_t cua ADC1
 */
void ADC1_IRQHandler(void)
{
    adc1_flag = true;
    /* Read conversion result to clear the conversion completed flag. */
    adc1ChanelValue = ADC12_GetChannelConversionValue( ADC1, ADC1_CHANEL_GROUP);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}
/**
 * @brief   Ham khoi tao ADC0
 *
 * @param   NONE
 * @retval  NONE
 */

void ADC0_Init()
{
    CLOCK_SetIpSrc( CLOCK_ADC0, kCLOCK_IpSrcSircAsync);
    EnableIRQ(ADC0_IRQn);

     /* Initialize ADC. */
    adc0ConfigStruct.clockSource                = CLOCK_SOURCE_ADC0;                    /* Cai dat xung clock cho bo ADC0 */
    adc0ConfigStruct.referenceVoltageSource     = kADC12_ReferenceVoltageSourceVref;    /* Cai dat dien ap tham chieu ADC0 */
    adc0ConfigStruct.resolution                 = kADC12_Resolution12Bit;               /* Cai dat do phan giai ADC0 - 12 bit */
    adc0ConfigStruct.clockDivider               = kADC12_ClockDivider2;                 /* Cai dat bo chia tan so clock - /2 */
    adc0ConfigStruct.sampleClockCount           = 4U;                                   /* Tan so dau vao /2 = 4MHz */
    adc0ConfigStruct.enableContinuousConversion = false;

    ADC12_Init(ADC0, &adc0ConfigStruct);
    /* Set to software trigger mode. */
    ADC12_EnableHardwareTrigger(ADC0, false);

    if (kStatus_Success != ADC12_DoAutoCalibration( ADC0))
    {
      Dbg_Println("Adc >> ADC0 calibration failed!\r\n");
    }

    adc0Channel0ConfigStruct.channelNumber = ADC0_CHANEL_MOTOR;                         /* Cau hinh kenh 0 bo ADC0 la chan nhan cam bien dong motor */
    adc0Channel0ConfigStruct.enableInterruptOnConversionCompleted = true;
    adc0Channel3ConfigStruct.channelNumber = ADC0_CHANEL_BATTERY;                       /* Cau hinh kenh 3 bo ADC0 la chan nhan nguon pin - khong su dung */
    adc0Channel3ConfigStruct.enableInterruptOnConversionCompleted = true;

    Pin_Init( PIN_ADC_MOTOR, PIN_INPUT_PULLDOWN);                                       /* Khoi tao chan dau vao kenh ADC0_0 */
    Pin_Init( PIN_ADC_BATTERY, PIN_INPUT_PULLDOWN);                                     /* Khoi tao chan dau vao kenh ADC0_3 */
}

/**
 * @brief   Ham khoi tao ADC1
 *
 * @param   NONE
 * @retval  NONE
 */
void ADC1_Init()
{
    CLOCK_SetIpSrc( CLOCK_ADC1, kCLOCK_IpSrcSircAsync);
    EnableIRQ(ADC1_IRQn);

     /* Initialize ADC. */
  //  ADC12_GetDefaultConfig(&adc1ConfigStruct);
    adc1ConfigStruct.clockSource                = CLOCK_SOURCE_ADC1;                    /* Cai dat xung clock cho bo ADC1 */
    adc1ConfigStruct.referenceVoltageSource     = kADC12_ReferenceVoltageSourceVref;    /* Cai dat dien ap tham chieu ADC1 */
    adc1ConfigStruct.resolution                 = kADC12_Resolution12Bit;               /* Cai dat do phan giai ADC1 - 12 bit */
    adc1ConfigStruct.clockDivider               = kADC12_ClockDivider2;                 /* Cai dat bo chia tan so clock - /2 */
    adc1ConfigStruct.sampleClockCount           = 4U;                                   /* Tan so dau vao /2 = 4MHz */
    adc1ConfigStruct.enableContinuousConversion = false;

    ADC12_Init(ADC1, &adc1ConfigStruct);
    /* Set to software trigger mode. */
    ADC12_EnableHardwareTrigger(ADC1, false);

    if (kStatus_Success != ADC12_DoAutoCalibration( ADC1))
    {
      Dbg_Println("Adc >> ADC1 calibration failed!\r\n");
    }

    adc1Channel0ConfigStruct.channelNumber = ADC1_CHANEL_PIN;                           /* Cau hinh kenh 0 bo ADC1 la chan bao co nguon dien */
    adc1Channel0ConfigStruct.enableInterruptOnConversionCompleted = true;
    adc1Channel1ConfigStruct.channelNumber = ADC1_CHANEL_NTC_P;                         /* Cau hinh kenh 1 bo ADC1 la chan + cam bien nhiet do NTC */
    adc1Channel1ConfigStruct.enableInterruptOnConversionCompleted = true;
    adc1Channel2ConfigStruct.channelNumber = ADC1_CHANEL_NTC_N;                         /* Cau hinh kenh 2 bo ADC1 la chan + cam bien nhiet do NTC */
    adc1Channel2ConfigStruct.enableInterruptOnConversionCompleted = true;

    Pin_Init(PIN_ADC_PIN, PIN_INPUT_PULLDOWN);                                          /* Khoi tao chan dau vao kenh ADC1_0 */
    Pin_Init(PIN_ADC_NTC_P, PIN_INPUT_PULLDOWN);                                        /* Khoi tao chan dau vao kenh ADC1_1 */
    Pin_Init(PIN_ADC_NTC_N, PIN_INPUT_PULLDOWN);                                        /* Khoi tao chan dau vao kenh ADC1_2 */
}
/**
 * @brief   Ham doc gia tri ADC tren cac kenh cua ADC0
 *
 * @param   uint8_t chanel -> Gia tri lua chon kenh ADC can doc
 * @retval  uint32_t adc0ChanelValue -> Gia tri 32bit cua kenh ADC0 can doc
 */
uint32_t getAdc0(uint8_t chanel)
{
    adc0_flag = false;
    if (chanel == 0 )                                                                   /* Neu kenh can doc la kenh 0 - ADC0 */
    {
        ADC12_SetChannelConfig(ADC0, ADC0_CHANEL_GROUP, &adc0Channel0ConfigStruct);     /* Cau hinh chan can doc la ADC0_0 */
        while(!adc0_flag);                                                              /* Cho den khi hoan thanh viec doc du lieu, ADC flag = 0 */
    }
    if (chanel == 3)                                                                    /* Neu kenh can doc la kenh 3 - ADC0 */
    {
        ADC12_SetChannelConfig(ADC0, ADC0_CHANEL_GROUP, &adc0Channel3ConfigStruct);     /* Cau hinh chan can doc la ADC0_3 */
        while(!adc0_flag);                                                              /* Cho den khi hoan thanh viec doc du lieu, ADC flag = 0 */
    }

    return adc0ChanelValue;
}
/**
 * @brief   Ham doc gia tri ADC tren cac kenh cua ADC1
 *
 * @param   uint8_t chanel -> Gia tri lua chon kenh ADC can doc
 * @retval  uint32_t adc1ChanelValue -> Gia tri 32bit cua kenh ADC1 can doc
 */
uint32_t getAdc1(uint8_t chanel)
{
    adc1_flag = false;
    if (chanel == 0 )                                                                   /* Neu kenh can doc la kenh 0 - ADC1 */
    {
        ADC12_SetChannelConfig(ADC1, ADC1_CHANEL_GROUP, &adc1Channel0ConfigStruct);     /* Cau hinh chan can doc la ADC1_0 */
        while(!adc1_flag);                                                              /* Cho den khi hoan thanh viec doc du lieu, ADC flag = 0 */
    }
    if (chanel == 1)                                                                    /* Neu kenh can doc la kenh 1 - ADC1 */
    {
        ADC12_SetChannelConfig(ADC1, ADC1_CHANEL_GROUP, &adc1Channel1ConfigStruct);     /* Cau hinh chan can doc la ADC1_1 */
        while(!adc1_flag);                                                              /* Cho den khi hoan thanh viec doc du lieu, ADC flag = 0 */
    }
    if (chanel == 2)                                                                    /* Neu kenh can doc la kenh 2 - ADC1 */
    {
        ADC12_SetChannelConfig(ADC1, ADC1_CHANEL_GROUP, &adc1Channel2ConfigStruct);     /* Cau hinh chan can doc la ADC1_2 */
        while(!adc1_flag);                                                              /* Cho den khi hoan thanh viec doc du lieu, ADC flag = 0 */
    }

    return adc1ChanelValue;
}

/************* The end configuration ADC ****************************************/