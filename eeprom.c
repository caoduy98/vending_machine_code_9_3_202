/** @file    eeprom.c
  * @author  
  * @version 
  * @brief   Khoi tao doc va ghi du lieu vao trong eeprom (cai dat va du lieu ban hang)
  */
#include "eeprom.h"
#include "ff.h"
#include "swi2c.h"

#define EEPROM_WR_ADD_FRAME     0xA2
#define EEPROM_RD_ADD_FRAME     0xA3

#define EEPROM_WR_ADD_SALE      0XA0
#define EEPROM_RD_ADD_SALE      0XA1


#define EEPROM_ADDRESS_MAX      65535   
#define SIZE_PAGE 128
#define MAX_WRITE_PAGE 10


/*comment*/
void EE_Init()
{
    
}
/**
 * @brief  Ham ghi mang du lieu vao EEPROM
 * 
 * @param  uint32_t address -> Dia chi bat dau
 * @param  uint8_t* data -> Con tro, tro den dia chi cua du lieu can ghi       
 * @param  uint32_t length -> Do dai mang du lieu can ghi
 * @retval bool EE_Write -> Trang thai ghi (False = loi, True = ok)
 */
bool EE_Write(uint32_t address, uint8_t* data, uint32_t length)
{
    /* Buffer save number byte of page */
	uint8_t nbPage[MAX_WRITE_PAGE] = {0}; // ch? gioi han ghi 10 page toi da la 1280byte - toi thieu duoc 1153
	uint16_t addPage[MAX_WRITE_PAGE] = {0};
	/* Check start page */
	uint16_t Start_Page = (address / 128);
	/* Check stop page */
	uint16_t Stop_Page = (address + length) / 128;
	/* Calculate number page */
	uint8_t NPage = Stop_Page - Start_Page + 1;
	if (NPage > MAX_WRITE_PAGE)
	{
		Dbg_Println("Eeprom >> Write Length over maximum 1");
		return false;
	}
	
	/* Calculate number byte of page */
	for (uint8_t index = 0; index < NPage; index++)
	{
		nbPage[index] = ((address / 128) + 1)*128 - address;
		addPage[index] =  address;
		address += nbPage[index];
	}
	
	for (uint8_t index = 0; index < NPage - 1; index++)
	{
		length -= nbPage[index];
	}
	nbPage[NPage - 1] = length;
	
	/* Write Process */
	for (uint8_t index = 0; index < NPage; index++)
	{
		/* Start I2C */
		SWI2C_Start(&g_softI2C0);
		/* Ghi dia chi EEPROM, chuc nang ghi */
		if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_WR_ADD_FRAME))
		{
			SWI2C_Stop(&g_softI2C0);
			
			return false;   /* Loi I2C */
		}
		/* Ghi dia chi, byte cao */
		if (!SWI2C_WriteByte(&g_softI2C0, addPage[index] >> 8))
		{
			SWI2C_Stop(&g_softI2C0);

			return false;   /* Loi I2C */
		}
		/* Ghi dia chi, byte thap */
		if (!SWI2C_WriteByte(&g_softI2C0, addPage[index]))
		{
			SWI2C_Stop(&g_softI2C0);
			return false;   /* Loi I2C */
		}
		
		/* Ghi toan bo du lieu */
		for (int i = 0; i < nbPage[index]; i++)
		{
			if (i < 128)
			{
				/* Ghi byte du lieu */
				if (!SWI2C_WriteByte(&g_softI2C0, *data++))
				{
					SWI2C_Stop(&g_softI2C0);
					return false;   /* Loi I2C */
				}
			}
		}
		
		/* Stop I2C */
		SWI2C_Stop(&g_softI2C0);    
		Delay(10);
	}
    
    return true;        /* Bao trang thai ghi hoan thanh */
}
/**
 * @brief  Ham doc mot mang du lieu tu EEPROM
 * 
 * @param  uint32_t address -> Dia chi bat dau      
 * @param  uint32_t length -> Do dai mang du lieu can doc
 * @retval uint8_t* data -> Con tro, tro den dia chi mang du lieu luu tru ket qua
 * @retval bool EE_Write -> Trang thai doc (False = loi, True = ok)
 */
bool EE_Read(uint32_t address, uint8_t* data, uint32_t length)
{
    for (int i = 0; i < length; i++)
    {
        /* Thuc hien lenh doc tung byte du lieu tu trong EEPROM, bat dau tu dia chi address */
        if (EE_ReadByte(address + i, &(data[i])) == false)
        {
            return false;       /* Doc loi */
        }
    }
    return true;        /* Hoan thanh viec doc EEPROM */
}

/**
 * @brief  Ham ghi 1 byte du lieu vao EEPROM
 * 
 * @param  uint32_t address -> Dia chi      
 * @param  uint8_t data -> Byte du lieu can ghi
 * @retval bool EE_WriteByte -> Trang thai ghi (False = loi, True = ok)
 */
bool EE_WriteByte(uint32_t address, uint8_t data)
{
    SWI2C_Start(&g_softI2C0);   /* Chay I2C */
    /* Ghi dia chi EEPROM, chuc nang ghi*/
    if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_WR_ADD_FRAME))
    {
        SWI2C_Stop(&g_softI2C0);
        
        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte cao */
    if (!SWI2C_WriteByte(&g_softI2C0, address >> 8))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte thap */
    if (!SWI2C_WriteByte(&g_softI2C0, address))
    {
        SWI2C_Stop(&g_softI2C0);
        return false;   /* Loi I2C */
    }
    /* Ghi byte du lieu */
    if (!SWI2C_WriteByte(&g_softI2C0, data))
    {
        SWI2C_Stop(&g_softI2C0);
        
        return false;   /* Loi I2C */
    }
    SWI2C_Stop(&g_softI2C0);    /* Dung I2C */
    Delay(6);
    return true;        /* Bao trang thai ghi hoan thanh */
}

/**
 * @brief  Ham doc du lieu tu EEPROM
 * 
 * @param  uint32_t address -> Dia chi     
 * @retval uint8_t *data -> Con tro, tro den du lieu nhan duoc 
 * @retval bool EE_ReadByte -> Trang thai doc (False = loi, True = ok)
 */
bool EE_ReadByte(uint32_t address, uint8_t *data)
{
    SWI2C_Start(&g_softI2C0);   /* Chay I2C */
    /* Ghi dia chi EEPROM, chuc nang ghi*/
    if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_WR_ADD_FRAME))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte cao */    
    if (!SWI2C_WriteByte(&g_softI2C0, address >> 8))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte thap */   
    if (!SWI2C_WriteByte(&g_softI2C0, address))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    SWI2C_Start(&g_softI2C0);   /* Chay lai I2C */
    /* Ghi dia chi EEPROM, chuc nang doc */
    if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_RD_ADD_FRAME))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Nhan du lieu vao bien con tro data, che do NACK */
    *data = SWI2C_ReadByte(&g_softI2C0, 0);
    SWI2C_Stop(&g_softI2C0);    /* Dung I2C */
    return true;        /* Bao trang thai doc hoan thanh */
}

/******************************************************************************
Sale EEPROM process 
******************************************************************************/
bool EE_Write_Sale(uint32_t address, uint8_t* data, uint32_t length)
{
    /* Buffer save number byte of page */
	uint8_t nbPage[MAX_WRITE_PAGE] = {0}; // ch? gioi han ghi 10 page toi da la 1280byte - toi thieu duoc 1153
	uint16_t addPage[MAX_WRITE_PAGE] = {0};
	/* Check start page */
	uint16_t Start_Page = (address / 128);
	/* Check stop page */
	uint16_t Stop_Page = (address + length) / 128;
	/* Calculate number page */
	uint8_t NPage = Stop_Page - Start_Page + 1;
	if (NPage > MAX_WRITE_PAGE)
	{
		Dbg_Println("Eeprom >> Write Length over maximum 2");
		return false;
	}
	
	/* Calculate number byte of page */
	for (uint8_t index = 0; index < NPage; index++)
	{
		nbPage[index] = ((address / 128) + 1)*128 - address;
		addPage[index] =  address;
		address += nbPage[index];
	}
	
	for (uint8_t index = 0; index < NPage - 1; index++)
	{
		length -= nbPage[index];
	}
	nbPage[NPage - 1] = length;
	
	/* Write Process */
	for (uint8_t index = 0; index < NPage; index++)
	{
		/* Start I2C */
		SWI2C_Start(&g_softI2C0);
		/* Ghi dia chi EEPROM, chuc nang ghi */
		if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_WR_ADD_SALE))
		{
			SWI2C_Stop(&g_softI2C0);
			
			return false;   /* Loi I2C */
		}
		/* Ghi dia chi, byte cao */
		if (!SWI2C_WriteByte(&g_softI2C0, addPage[index] >> 8))
		{
			SWI2C_Stop(&g_softI2C0);

			return false;   /* Loi I2C */
		}
		/* Ghi dia chi, byte thap */
		if (!SWI2C_WriteByte(&g_softI2C0, addPage[index]))
		{
			SWI2C_Stop(&g_softI2C0);
			return false;   /* Loi I2C */
		}
		
		/* Ghi toan bo du lieu */
		for (int i = 0; i < nbPage[index]; i++)
		{
			if (i < 128)
			{
				/* Ghi byte du lieu */
				if (!SWI2C_WriteByte(&g_softI2C0, *data++))
				{
					SWI2C_Stop(&g_softI2C0);
					return false;   /* Loi I2C */
				}
			}
		}
		
		/* Stop I2C */
		SWI2C_Stop(&g_softI2C0);    
		Delay(10);
	}
    
    return true;        /* Bao trang thai ghi hoan thanh */
}
/****************************************************************************/
bool EE_Read_Sale(uint32_t address, uint8_t* data, uint32_t length)
{
    for (int i = 0; i < length; i++)
    {
        /* Thuc hien lenh doc tung byte du lieu tu trong EEPROM, bat dau tu dia chi address */
        if (EE_ReadByte_Sale(address + i, &(data[i])) == false)
        {
            return false;       /* Doc loi */
        }
    }
    return true;        /* Hoan thanh viec doc EEPROM */
}
/****************************************************************************/
bool EE_WriteByte_Sale(uint32_t address, uint8_t data)
{
    SWI2C_Start(&g_softI2C0);   /* Chay I2C */
    /* Ghi dia chi EEPROM, chuc nang ghi*/
    if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_WR_ADD_SALE))
    {
        SWI2C_Stop(&g_softI2C0);
        
        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte cao */
    if (!SWI2C_WriteByte(&g_softI2C0, address >> 8))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte thap */
    if (!SWI2C_WriteByte(&g_softI2C0, address))
    {
        SWI2C_Stop(&g_softI2C0);
        return false;   /* Loi I2C */
    }
    /* Ghi byte du lieu */
    if (!SWI2C_WriteByte(&g_softI2C0, data))
    {
        SWI2C_Stop(&g_softI2C0);
        
        return false;   /* Loi I2C */
    }
    SWI2C_Stop(&g_softI2C0);    /* Dung I2C */
    Delay(6);
    return true;        /* Bao trang thai ghi hoan thanh */
}
/****************************************************************************/
bool EE_ReadByte_Sale(uint32_t address, uint8_t *data)
{
    SWI2C_Start(&g_softI2C0);   /* Chay I2C */
    /* Ghi dia chi EEPROM, chuc nang ghi*/
    if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_WR_ADD_SALE))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte cao */    
    if (!SWI2C_WriteByte(&g_softI2C0, address >> 8))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Ghi dia chi, byte thap */   
    if (!SWI2C_WriteByte(&g_softI2C0, address))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    SWI2C_Start(&g_softI2C0);   /* Chay lai I2C */
    /* Ghi dia chi EEPROM, chuc nang doc */
    if (!SWI2C_WriteByte(&g_softI2C0, EEPROM_RD_ADD_SALE))
    {
        SWI2C_Stop(&g_softI2C0);

        return false;   /* Loi I2C */
    }
    /* Nhan du lieu vao bien con tro data, che do NACK */
    *data = SWI2C_ReadByte(&g_softI2C0, 0);
    SWI2C_Stop(&g_softI2C0);    /* Dung I2C */
    return true;        /* Bao trang thai doc hoan thanh */
}

