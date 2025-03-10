#ifndef MOTOR_H
#define MOTOR_H

#include "pin_num.h"
#include <stdbool.h>
#include <stdint.h>

#define MOTOR_NOT_STOP_FLAG_SET      0x01
#define MOTOR_NOT_STOP_FLAG_CLEAR    0x00

#define DO0         PB3             /* Control motor */
#define DO2         PC14            /* Control trapdoor */
#define DO3         PC15            /* Control provide motor */
#define FX1         PE16            /* Cam bien het huong */
#define LED         PA8            /* Tat led khi het huong */

#define DO5         PC17            /* Control provide motor Swap */

void Motor_Init(void);
void MotorRun(void);
void MotorStop();
void MotorProvideRun(void);
void MotorProvideStop(void);
void Trapdoor_Open(void);
void Trapdoor_Off(void);

void SensorOutStock_Init(void);
bool SensorOurStock_IsEmpty(void);

void MotorProvideRunSwap(void);
void MotorProvideStopSwap(void);
#endif