
#include "swi2c.h"


#define Q_DEL       for (uint32_t delay_index = 0; delay_index < 50; delay_index++);
#define H_DEL       for (uint32_t delay_index = 0; delay_index < 50; delay_index++);

/**
 * @brief  Ham khoi tao I2C mem
 * 
 * @param  swi2c_t* obj -> Con tro, tro den cau hinh chan I2C               
 * @retval NONE     
 */
void SWI2C_Init(swi2c_t* obj)
{
    DEV_ASSERT(obj != NULL);
    Pin_Init(obj->sclPin, PIN_INPUT);           /*Cau hinh SCL là chan vao*/           
    Pin_Init(obj->sdaPin, PIN_INPUT);           /*Cau hinh SDA là chan vao*/
}

/**
 * @brief  Ham khoi dong chuan I2C
 * 
 * @param  swi2c_t* obj -> Con tro, tro den cau hinh chan I2C               
 * @retval NONE     
 */
void SWI2C_Start(swi2c_t* obj)
{
    DEV_ASSERT(obj != NULL);
    
    Pin_SetDir(obj->sclPin, PIN_INPUT);         /*Cau hinh SCL là chan vao*/
    H_DEL;

    Pin_SetDir(obj->sdaPin, PIN_OUTPUT);        /*Cau hinh SDA la dau ra, muc thap */                     
    Pin_Write(obj->sdaPin, 0);
    H_DEL;
}

/**
 * @brief  Ham dung chuan I2C
 * 
 * @param  swi2c_t* obj -> Con tro, tro den cau hinh chan I2C               
 * @retval NONE     
 */
void SWI2C_Stop(swi2c_t* obj)
{
    DEV_ASSERT(obj != NULL);
    
    Pin_SetDir(obj->sdaPin, PIN_OUTPUT);        /*Cau hinh SDA la dau ra, muc thap */
    Pin_Write(obj->sdaPin, 0);
    H_DEL;
    Pin_SetDir(obj->sclPin, PIN_INPUT);         /*Cau hinh SCL là chan vao*/
    Q_DEL;
    Pin_SetDir(obj->sdaPin, PIN_INPUT);         /*Cau hinh SDA là chan vao*/
    H_DEL;
}

/**
 * @brief  Ham khoi dong la chuan I2C
 * 
 * @param  swi2c_t* obj -> Con tro, tro den cau hinh chan I2C               
 * @retval NONE     
 */
void SWI2C_Restart(swi2c_t* obj)
{
    DEV_ASSERT(obj != NULL);
}

/**
 * @brief  Ham ghi 1 byte du lieu qua chuan I2C
 * 
 * @param  swi2c_t* obj -> Con tro, tro den cau hinh chan I2C
 * @param  uint8_t data -> Du lieu can ghi               
 * @retval NONE     
 */
bool SWI2C_WriteByte(swi2c_t* obj, uint8_t data)
{
    DEV_ASSERT(obj != NULL);
    
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        Pin_SetDir(obj->sclPin, PIN_OUTPUT);    /* SCL is low */
        Pin_Write(obj->sclPin, 0);
        Q_DEL;

        if (data & 0x80)
        {
            Pin_SetDir(obj->sdaPin, PIN_INPUT);     /* SDA is high */
        }
        else
        {
            Pin_SetDir(obj->sdaPin, PIN_OUTPUT);     /* SDA is low */
            Pin_Write(obj->sdaPin, 0);
        }

        H_DEL;

        Pin_SetDir(obj->sclPin, PIN_INPUT);     /* SCL is high */
        H_DEL;

        //while ((SCLPIN & (1 << SCL)) == 0);     /* Wait for the SCL to be high */

        data = data << 1;
    }

    //The 9th clock (ACK Phase)
    Pin_SetDir(obj->sclPin, PIN_OUTPUT);    /* SCL is low */
    Pin_Write(obj->sclPin, 0);
    Q_DEL;

    Pin_SetDir(obj->sdaPin, PIN_INPUT);     /* SDA is high */
    H_DEL;

    Pin_SetDir(obj->sclPin, PIN_INPUT);     /* SCL is high */
    H_DEL;

    bool ack = false;
    if (Pin_Read(obj->sdaPin) == 0)
    {
        ack = true;
    }

    Pin_SetDir(obj->sclPin, PIN_OUTPUT);    /* SCL is low */
    Pin_Write(obj->sclPin, 0);
    H_DEL;

    return ack;
}

/**
 * @brief  Ham ghi nhan 1 byte du lieu qua chuan I2C
 * 
 * @param  swi2c_t* obj -> Con tro, tro den cau hinh chan I2C
 * @param  bool ack -> Lua chon ACK hay NACK              
 * @retval uint8_t SWI2C_ReadByt -> Du lieu nhan ve tu I2C     
 */
uint8_t SWI2C_ReadByte(swi2c_t* obj, bool ack)
{
    DEV_ASSERT(obj != NULL);
    
    uint8_t data = 0x00;
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        Pin_SetDir(obj->sclPin, PIN_OUTPUT);    /* SCL is low */
        Pin_Write(obj->sclPin, 0);
        H_DEL;
        Pin_SetDir(obj->sclPin, PIN_INPUT);     /* SCL is high */
        H_DEL;

        //while ((SCLPIN & (1<<SCL))==0);

        if (Pin_Read(obj->sdaPin))
        {
            data |= (0x80 >> i);
        }
    }

    Pin_SetDir(obj->sclPin, PIN_OUTPUT);    /* SCL is low */
    Pin_Write(obj->sclPin, 0);
    Q_DEL;                      //Soft_I2C_Put_Ack

    if (ack)
    {
        Pin_SetDir(obj->sdaPin, PIN_OUTPUT);     /* SDA is low */
        Pin_Write(obj->sdaPin, 0);
    }
    else
    {
        Pin_SetDir(obj->sdaPin, PIN_INPUT);     /* SDA is high */
    }
    H_DEL;

    Pin_SetDir(obj->sclPin, PIN_INPUT);     /* SCL is high */
    H_DEL;

    Pin_SetDir(obj->sclPin, PIN_OUTPUT);    /* SCL is low */
    Pin_Write(obj->sclPin, 0);
    H_DEL;

    return data;
}


