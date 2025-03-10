
#include "platform.h"
#include "spi.h"
#include "fsl_lpspi.h"

static LPSPI_Type* g_lpspiBase[] = LPSPI_BASE_PTRS;
static clock_ip_name_t g_lpspiClockName[] = { kCLOCK_Lpspi0, kCLOCK_Lpspi1 };


void Spi_Init(spi_t* obj, spi_mode_t mode)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SPI_PORT_NUM);
    
    obj->mode = mode;
    if (obj->mode == SPI_MASTER)
    {
        lpspi_master_config_t masterConfig;
        
        LPSPI_MasterGetDefaultConfig(&masterConfig);
        LPSPI_MasterInit(g_lpspiBase[obj->port], &masterConfig, CLOCK_GetIpFreq(g_lpspiClockName[obj->port]));
        LPSPI_SetDummyData(g_lpspiBase[obj->port], 0xFF);
    }
    else
    {
        
    }
}

void Spi_SetMode(spi_t* obj, spi_mode_t mode)
{
    
}

void Spi_SetSpeed(spi_t* obj, uint32_t speed)
{
    
}

void Spi_SetBitOrder(spi_t* obj, spi_bit_order_t bitOrder)
{
    
}

void Spi_SetDataMode(spi_t* obj, spi_data_mode_t mode)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SPI_PORT_NUM);
    
    LPSPI_Type* base = g_lpspiBase[obj->port];
    
    if (mode == SPI_DATA_MODE0)
    {
        base->TCR &= ~LPSPI_TCR_CPOL_MASK;
        base->TCR &= ~LPSPI_TCR_CPHA_MASK;
    }
    else if (mode == SPI_DATA_MODE1)
    {
        base->TCR &= ~LPSPI_TCR_CPOL_MASK;
        base->TCR |= LPSPI_TCR_CPHA_MASK;
    }
    else if (mode == SPI_DATA_MODE2)
    {
        base->TCR |= LPSPI_TCR_CPOL_MASK;
        base->TCR &= ~LPSPI_TCR_CPHA_MASK;
    }
    else if (mode == SPI_DATA_MODE3)
    {
        base->TCR |= LPSPI_TCR_CPOL_MASK;
        base->TCR |= LPSPI_TCR_CPHA_MASK;
    }
}    

void Spi_Transfer(spi_t* obj, uint8_t* txData, uint8_t* rxData, uint32_t size)
{
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SPI_PORT_NUM);
    
    if (obj->mode == SPI_MASTER)
    {
        lpspi_transfer_t masterXfer;
        masterXfer.txData = txData;
        masterXfer.rxData = rxData;
        masterXfer.dataSize = size;
        LPSPI_MasterTransferBlocking(g_lpspiBase[obj->port], &masterXfer);
    }
    else
    {
        
    }
}

uint8_t Spi_TransferByte(spi_t* obj, uint8_t byte)
{
    uint8_t rxByte = 0;
    
    DEV_ASSERT(obj != NULL);
    DEV_ASSERT(obj->port < SPI_PORT_NUM);
    
    if (obj->mode == SPI_MASTER)
    {
        lpspi_transfer_t masterXfer;
        masterXfer.txData = &byte;
        masterXfer.rxData = &rxByte;
        masterXfer.dataSize = 1;
        LPSPI_MasterTransferBlocking(g_lpspiBase[obj->port], &masterXfer);
    }
    else
    {
        
    }
    
    return rxByte;
}
