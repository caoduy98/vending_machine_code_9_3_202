
#include "platform.h"
#include "serial.h"
#include "fsl_lpuart.h"


static LPUART_Type* g_lpuartBase[] = LPUART_BASE_PTRS;
static clock_ip_name_t g_lpuartClockIP[] = { kCLOCK_Lpuart0, kCLOCK_Lpuart1, kCLOCK_Lpuart2 };

/**
 * @brief  Ham khoi tao cong truyen thong UART
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART 
 * @param  uint32_t baudrate -> Toc do baud cau hinh cho cong UART                 
 * @retval NONE     
 */
void Serial_Init(serial_t* obj, uint32_t baudrate)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);

    LPUART_Type* base = g_lpuartBase[obj->port];
    
    lpuart_config_t config;
    
    /* Enable clock */
    CLOCK_SetIpSrc(g_lpuartClockIP[obj->port], kCLOCK_IpSrcSysOscAsync);
    
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = baudrate;
    config.enableTx = true;
    config.enableRx = true;
    
    if (obj->port == 1) {   //Them 2/3/2025
       config.stopBitCount = kLPUART_TwoStopBit; //Them 2/3/2025
    }  //Them 2/3/2025

    LPUART_Init(base, &config, CLOCK_GetFreq(kCLOCK_ScgSysOscClk));
    
    /* Enable RX interrupt. */
    if (obj->privateObj.useRxBuffer == true)
    {
        LPUART_EnableInterrupts(base, 
                                kLPUART_RxDataRegFullInterruptEnable |
                                kLPUART_RxOverrunInterruptEnable |
                                kLPUART_NoiseErrorInterruptEnable |
                                kLPUART_FramingErrorInterruptEnable |
                                kLPUART_ParityErrorInterruptEnable);
        NVIC_SetPriority(LPUART2_IRQn, 1);
        NVIC_SetPriority(LPUART1_IRQn, 1);
        EnableIRQ(LPUART2_IRQn);
        EnableIRQ(LPUART1_IRQn);
    }
}

/**
 * @brief  Ham cai dat toc do baud cong UART
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART 
 * @param  uint32_t baudrate -> Toc do baud cau hinh cho cong UART                 
 * @retval NONE     
 */
void Serial_SetBaudrate(serial_t* obj, uint32_t baudrate)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    LPUART_Type* base = g_lpuartBase[obj->port];
    LPUART_SetBaudRate(base, baudrate, CLOCK_GetFreq(kCLOCK_ScgSysOscClk));
}
/**
 * @brief  Ham gui du lieu 1 byte qua cong UART
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART 
 * @param  uint8_t byte -> Du lieu gui qua cong UART                
 * @retval NONE     
 */
void Serial_WriteByte(serial_t* obj, uint8_t byte)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    LPUART_Type* base = g_lpuartBase[obj->port]; 
    LPUART_WriteBlocking(base, &byte, 1);
}

/**
 * @brief  Ham gui du lieu 1 chuoi ki tu (Mang) qua cong UART
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART 
 * @param  uint8_t* data -> Chuoi ki tu (Mang)   
 * @param  uint32_t length -> Do dai cua chuoi ki tu (Mang)
 * @retval NONE     
 */
void Serial_Write(serial_t* obj, uint8_t* data, uint32_t length)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    LPUART_Type* base = g_lpuartBase[obj->port];
    LPUART_WriteBlocking(base, data, length);
}


/* Hien tai khong su dung ham nay */
void Serial_InstallCallback(serial_t* obj, serial_callbacl_t callback, void* param)
{
	DEV_ASSERT(obj != NULL);
        DEV_ASSERT(obj->port < SERIAL_PORT_NUM);

	obj->privateObj.callback = callback;
	obj->privateObj.param = param;
}

/**
 * @brief  Ham kiem tra xem co du lieu, va do dai du lieu trong bo dem (RxBuffer) 
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART 
 * @retval uint32_t Serial_Available -> Do dai cua du lieu can doc    
 */

uint32_t Serial_Available(serial_t* obj)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    LPUART_Type* base = g_lpuartBase[obj->port];

    uint32_t bytesToRead = 0;
    if (obj->privateObj.useRxBuffer)
    {
        bytesToRead = ((unsigned int)(obj->rxBufferSize + obj->privateObj.rxHead - obj->privateObj.rxTail)) % (obj->rxBufferSize);
    }
    else
    {
        if ((base->STAT & LPUART_STAT_RDRF_MASK))
        {
            bytesToRead = 1;
        }
    }

    return bytesToRead;
}

/**
 * @brief  Ham nhan tung byte du lieu tu bo dem (rxBuffer)
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART 
 * @retval int Serial_ReadByte -> Du lieu nhan duoc tu bo dem  
 */
int Serial_ReadByte(serial_t* obj)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    LPUART_Type* base = g_lpuartBase[obj->port];
    uint32_t status;
    int c = -1;
    
    status = base->STAT;
    if (obj->privateObj.useRxBuffer)
    {
        if (obj->privateObj.rxHead != obj->privateObj.rxTail)
        {
            c = obj->rxBuffer[obj->privateObj.rxTail];
            obj->privateObj.rxTail = (obj->privateObj.rxTail + 1) % obj->rxBufferSize;
        }
    }
    else if (status & LPUART_STAT_OR_MASK)      /* Over run error */
    {
        base->STAT |= LPUART_STAT_OR_MASK;
    }
    else if (status & LPUART_STAT_NF_MASK)      /* Noise error */
    {
        base->STAT |= LPUART_STAT_NF_MASK;
    }
    else if (status & LPUART_STAT_FE_MASK)      /* Framing error */
    {
        base->STAT |= LPUART_STAT_FE_MASK;
    }
    else if (status & LPUART_STAT_PF_MASK)      /* Parity error */
    {
        base->STAT |= LPUART_STAT_PF_MASK;
    }
    else if (status & LPUART_STAT_RDRF_MASK)
    {
        c = base->DATA;
    }

    return c;
}

/**
 * @brief  Ham kiem tra du lieu trong bo dem nhan (RxBuffer)
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART
 * @retval int Serial_PeekByte -> Du lieu nhan duoc tu bo dem   
 */
int Serial_PeekByte(serial_t* obj)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);

    int c = -1;
    if (obj->privateObj.useRxBuffer)
    {
        if (obj->privateObj.rxHead != obj->privateObj.rxTail)
        {
            c = obj->rxBuffer[obj->privateObj.rxTail];
        }
    }

    return c;
}
/**
 * @brief  Ham tim mot chuoi ki tu trong bo dem
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART
 * @param  char* str -> Chuoi ki tu can tim 
 * @retval int Serial_ReadByte -> Du lieu nhan duoc tu cong UART    
 */
char* Serial_FindString(serial_t* obj, char* str)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    uint8_t len = strlen(str);
    char* posBuf = NULL;
    bool findFlag = false;
    for(uint32_t index = obj->privateObj.rxTail; index < obj->rxBufferSize; index++)
    {
      if(obj->rxBuffer[index] == str[0])
      {
	findFlag = true;
	posBuf = (char*)obj->rxBuffer + index;
	for(uint8_t count = 0; count < len; count++)
	{
	  if(obj->rxBuffer[index + count] != str[count])
	  {
	    findFlag = false;
	  }
	}
	
	if(findFlag) return posBuf;
      }
    }
    return 0;
//    return strstr(&obj->rxBuffer[obj->privateObj.rxTail], str);                 /* strtr -> ham tim kiem chuoi nho trong chuoi */
    /* Ket qua tra ve la NULL neu khong tim thay */
}

/**
 * @brief  Ham xoa du lieu trong bo dem nhan (RxBufer)
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART
 * @retval NONE  
 */
void Serial_ClearRxBuffer(serial_t* obj)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    obj->privateObj.rxTail = 0;
    obj->privateObj.rxHead = 0;
    
    for (int i = 0; i < obj->rxBufferSize; i++)
    {
        obj->rxBuffer[i] = 0;
    }
}

/* Interrupt handler */
/**
 * @brief  Ham truyen du lieu tu cong UART vao bo dem nhan (RxBufer)
 * 
 * @param  serial_t* obj -> Con tro den cau hinh cong UART
 * @param  uint8_t rxData -> Du lieu truyen vao bo dem
 * @retval NONE  
 */
void Serial_RxHandler(serial_t* obj, uint8_t rxData)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SERIAL_PORT_NUM);
    
    obj->rxBuffer[obj->privateObj.rxHead] = rxData;
    obj->privateObj.rxHead = (obj->privateObj.rxHead + 1) % obj->rxBufferSize;
    obj->rxBuffer[obj->privateObj.rxHead] = 0;
}
