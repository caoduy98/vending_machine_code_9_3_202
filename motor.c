#include "motor.h"
#include "pin.h"
#include "tickcount.h"
#include "adc.h"
#include "dbg_console.h"
#include "stdio.h"
#include "peripheral.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_MOTOR
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif

uint8_t motorNotStop = MOTOR_NOT_STOP_FLAG_CLEAR;
/**
 * @brief  Ham tao tre us
 *
 * @param  uint16_t us -> Thoi gian tre us
 * @retval NONE
 */
void DelayUS(uint16_t us)
{
    uint32_t ti = 48 * us / 12;
    for (; ti > 0; ti--);
}
/**
 * @brief  Ham dieu khien dong co dung
 *
 * @param  NONE
 * @retval NONE
 */
void MotorStop() {
    Pin_Write(DO0, 0);
}

/**
 * @brief  Ham dieu khien dong co chay
 *
 * @param  NONE
 * @retval NONE
 */
void MotorRun(void) {
    Pin_Write(DO0, 1);
}

/**
 * @brief  Ham dieu khien dong co cap huong chay
 *
 * @param  NONE
 * @retval NONE
 */
void MotorProvideRun(void) {
    Pin_Write(DO3, 1);
}

/**
 * @brief  Ham dieu khien dong co cap huong dung
 *
 * @param  NONE
 * @retval NONE
 */
void MotorProvideStop(void) {
    Pin_Write(DO3, 0);
}

/**
 * @brief  Ham khoi tao cac chan I/O dieu khien dong co
 *
 * @param  NONE
 * @retval NONE
 */
void Motor_Init()
{
    Pin_Init(DO2,PIN_OUTPUT);
    Pin_Init(DO0, PIN_OUTPUT);
    Pin_Init(DO3, PIN_OUTPUT);
    Pin_Init(DO5, PIN_OUTPUT);
    Pin_Init(LED, PIN_OUTPUT); //moi them
    Pin_Write(DO2, 0);
    Pin_Write(DO0, 0);
    Pin_Write(DO3, 0);
    Pin_Write(DO5, 1);
    Pin_Write(LED, 0); //moi them
}

/**
 * @brief  Ham dieu khien cuon hut day huong
 *
 * @param  NONE
 * @retval NONE
 */
void Trapdoor_Open(void) {
    Pin_Write(DO2, 1);
}

/**
 * @brief  Ham dieu khien cuon hut thu lai
 *
 * @param  NONE
 * @retval NONE
 */
void Trapdoor_Off(void) {
    Pin_Write(DO2, 0);
}


/**
 * @brief  Ham khoi tao cac i/o cam bien so luong huong
 *
 * @param  NONE
 * @retval NONE
 */
void SensorOutStock_Init(void) {
    Pin_Init(FX1,PIN_INPUT_PULLUP);
}


/**
 * @brief  Ham check het huong
 *
 * @param  NONE
 * @retval bool -  true neu het huong, false neu con huong
 */
bool SensorOurStock_IsEmpty(void) {
    if(Pin_Read(FX1) == 1) {
        return false;
    }
    return true;
}

/**
 * @brief  Ham dieu khien cuon hut day huong
 *
 * @param  NONE
 * @retval NONE
 */
void MotorProvideRunSwap(void) {
    Pin_Write(DO5, 1);
}

/**
 * @brief  Ham dieu khien cuon hut thu lai
 *
 * @param  NONE
 * @retval NONE
 */
void MotorProvideStopSwap(void) {
    Pin_Write(DO5, 0);
}
