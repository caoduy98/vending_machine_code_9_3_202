
#ifndef EEPROM_H
#define EEPROM_H

#include "platform.h"

void EE_Init();
bool EE_Write(uint32_t address, uint8_t* data, uint32_t length);
bool EE_WriteByte(uint32_t address, uint8_t data);
bool EE_Read(uint32_t address, uint8_t* data, uint32_t length);
bool EE_ReadByte(uint32_t address, uint8_t* data);
bool EE_Write_Sale(uint32_t address, uint8_t* data, uint32_t length);
bool EE_Read_Sale(uint32_t address, uint8_t* data, uint32_t length);
bool EE_WriteByte_Sale(uint32_t address, uint8_t data);
bool EE_ReadByte_Sale(uint32_t address, uint8_t *data);

#endif  /* EEPROM_H */
