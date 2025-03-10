
#include "peripheral.h"
#include "Crc8bit.h"


/*****************************************************************************/

uint8_t check_crc8bit (uint8_t *data, uint8_t dataLength)
{
  uint8_t uIndex,i;
  uint8_t crc = 0xFF;
  for(i = 0;i < dataLength; i++)
  {
      uIndex = crc ^ *data++;
      crc = modbus_auchCRCLo[uIndex];
  }
  return(crc);
}
/*****************************************************************************/