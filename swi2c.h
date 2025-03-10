
#ifndef SW_I2C_H
#define SW_I2C_H

#include "platform.h"


typedef struct
{
    int sclPin;
    int sdaPin;
} swi2c_t;


void SWI2C_Init(swi2c_t* obj);
void SWI2C_Start(swi2c_t* obj);
void SWI2C_Stop(swi2c_t* obj);
void SWI2C_Restart(swi2c_t* obj);
bool SWI2C_WriteByte(swi2c_t* obj, uint8_t data);
uint8_t SWI2C_ReadByte(swi2c_t* obj, bool ack);

extern swi2c_t g_softI2C0;

#endif  /* SW_I2C_H */
