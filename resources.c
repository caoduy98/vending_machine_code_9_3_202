/** @file    resources.c
  * @author
  * @version
  * @brief   Cung cap cac ham thong ke doanh so ban hang theo nam thang ngay
  */
#include "platform.h"
#include "resources.h"
#include "ff.h"
#include "peripheral.h"
#include "Crc8bit.h"
#include "eeprom.h"
#include "gsm.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_RESOURCE
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
extern void SendSDCardErrorFrame();
extern void SendSDCardResumeFrame();

static uint32_t g_eepromsalepointer = 0;
static uint32_t g_eepromYearsalepointer = 0;

typedef struct
{
    uint8_t Header[2];
    uint8_t Date;
    uint8_t Month;
    uint8_t Year;
    uint8_t SaleValueH;
    uint8_t SaleValueL;
    uint8_t numbersaleH;
    uint8_t numbersaleL;
    uint8_t crc8bit;
} Sale_t;

typedef struct
{
    uint16_t Hearder;
    uint16_t Year;
    uint16_t SaleValueH;
    uint16_t SaleValueL;
    uint16_t numbersaleH;
    uint16_t numbersaleL;
    uint8_t crc8bit;
} SaleYear_t;

/*****************************************************************************/
bool Resource_SaveSalestoEeprom (int sales)
{
    int date,month,year;    
    uint16_t failCount = 0;
    uint8_t crc_readRom = 0;
    uint32_t total_saleday = 0;
    uint32_t total_numbersaleday = 0;
    uint8_t state_readSale = 0;
    Sale_t ReadSaleformRom;
    Rtc_GetCurrentDate(&date, &month, &year);
    Resource_SaveSalesYeartoEeprom(sales);
    ReadEepromsalepointer();               
    while(state_readSale == 0)
    {
        if(EE_Read_Sale(g_eepromsalepointer,&ReadSaleformRom,10) == true)
        {  
            crc_readRom = check_crc8bit(&ReadSaleformRom,9);
            if(crc_readRom == ReadSaleformRom.crc8bit)
            {
                state_readSale = 1;
                Dbg_Println("Resources >> DaylySale state 1");
            }
            else 
            {
                if((ReadSaleformRom.Date == 0xff)&&(ReadSaleformRom.Month == 0xff)&&
                  (ReadSaleformRom.Year == 0xff))
                {
                    state_readSale = 2;
                    Dbg_Println("Resources >> DaylySale state 2");
                }
                else 
                {
                    failCount ++;
                    if(failCount > 10)
                    {
                        state_readSale = 3;
                        Dbg_Println("Resources >> DaylySale state 3");
                    }
                }
            }
        }
        else 
        {
            failCount ++;
            if(failCount > 10)
            {
                Dbg_Println("Resources >> Read Sale Dayly Error!");
                return false;
            }
        }
    }
    if(state_readSale == 1)
    {
        if((date == ReadSaleformRom.Date)&&(month == ReadSaleformRom.Month)&&(year == ReadSaleformRom.Year))
        {
            total_saleday = (sales/1000) + (((0x00FF&ReadSaleformRom.SaleValueH)<< 8)|ReadSaleformRom.SaleValueL);
            total_numbersaleday = (((0x00FF&ReadSaleformRom.numbersaleH)<< 8)|ReadSaleformRom.numbersaleL) + 1;
            if(SaveTotalDaylySaletoEerom(total_saleday,total_numbersaleday) == true)
            {
                return true;
            }
            else 
            {
                Dbg_Println("Resources >> Save Dayly sale Error ");
                return false;
            }
        }    
        else
        {
            CreatTimeRequestFrame();
            total_saleday = sales/1000;
            total_numbersaleday = 1;
            g_eepromsalepointer += 10;
            if(g_eepromsalepointer > EEPROM_SALE_STOP_DAY)
            {
                g_eepromsalepointer = EEPROM_SALE_START_DAY;
            }
            SaveEepromsalepointer();
            Delay(50);
            if(SaveTotalDaylySaletoEerom(total_saleday,total_numbersaleday) == true)
            {   
                return true;   
            }
            else 
            {
                Dbg_Println("Resources >> Save Dayly sale Error ");
                return false;
            }
        }
    }
    if(state_readSale == 2)
    {
        total_saleday = sales/1000;
        total_numbersaleday = 1;
        if(SaveTotalDaylySaletoEerom(total_saleday,total_numbersaleday) == true)
        {   
            return true;
        }
        else 
        {
            Dbg_Println("Resources >> Save Dayly sale Error ");
            return false;  
        }
    }
    
    if(state_readSale == 3)
    {
        total_saleday = sales/1000;
        total_numbersaleday = 1;
        g_eepromsalepointer += 10;
        if(g_eepromsalepointer > EEPROM_SALE_STOP_DAY)
        {
            g_eepromsalepointer = EEPROM_SALE_START_DAY;
        }
        SaveEepromsalepointer();
        Delay(50);
        if(SaveTotalDaylySaletoEerom(total_saleday,total_numbersaleday) == true)
        {   
            Dbg_Println("Resources >> Save ST3 sale ok ");
            return true;
        }
        else return false;
    }
    return false;
}
/*****************************************************************************/ 
bool Resource_SaveSalesYeartoEeprom (int sales)
{
    int date,month,year;    
    uint16_t failCount = 0;
    uint8_t crc_readRom = 0;
    uint32_t total_saleYear = 0;
    uint32_t total_numbersaleYear = 0;
    uint8_t state_readSale = 0;
    SaleYear_t ReadSaleYearformRom;
    
    Rtc_GetCurrentDate(&date, &month, &year);
    ReadEepromYearsalepointer();
    
    while(state_readSale == 0)
    {
        if(EE_Read_Sale(g_eepromYearsalepointer,&ReadSaleYearformRom,13) == true)
        {  
            crc_readRom = check_crc8bit(&ReadSaleYearformRom,12);
            if(crc_readRom == ReadSaleYearformRom.crc8bit)
            {
                state_readSale = 1;
                Dbg_Println("Resources >> YearlySale state 1");
            }
            else 
            {
                if(ReadSaleYearformRom.Year == 0xFFFF)
                {
                    state_readSale = 2;
                    Dbg_Println("Resources >> YearlySale state 2");
                }
                else 
                {
                    failCount ++;
                    if(failCount > 10)
                    {
                        state_readSale = 2;
                        Dbg_Println("YearlySale state 3");
                    }
                    
                }
            }
        }
        else 
        {
            failCount ++;
            if(failCount > 10)
            {
                Dbg_Println("Resources >> Read Sale Yearly Error!");
                return false;
            }
        }
    }
    if(state_readSale == 1)
    {
        if(year == ReadSaleYearformRom.Year)
        {
            total_saleYear = (sales/1000) + (((0x0000FFFF&ReadSaleYearformRom.SaleValueH)<< 16)|ReadSaleYearformRom.SaleValueL);
            total_numbersaleYear = (((0x0000FFFF&ReadSaleYearformRom.numbersaleH)<< 16)|ReadSaleYearformRom.numbersaleL) + 1;
            if(SaveTotalYearlySaletoEerom(total_saleYear,total_numbersaleYear) == true)
            {
                return true;
            }
            else 
            {
                Dbg_Println("Resources >> Save Total Yearly sale Error!");
                return false;
            }
        }    
        else
        {
            total_saleYear = sales/1000;
            total_numbersaleYear = 1;
            g_eepromYearsalepointer += 13;
            if(g_eepromYearsalepointer > EEPROM_SALE_STOP_YEAR)
            {
                g_eepromYearsalepointer = EEPROM_SALE_START_YEAR;
            }
            SaveEepromYearsalepointer();
            Delay(50);
            if(SaveTotalYearlySaletoEerom(total_saleYear,total_numbersaleYear) == true)
            {   
                return true;   
            }
            else 
            {
                Dbg_Println("Resources >> Save Total Yearly sale Error!");
                return false;
            }
        }
    }
    if(state_readSale == 2)
    {
        total_saleYear = sales/1000;
        total_numbersaleYear = 1;
        if(SaveTotalYearlySaletoEerom(total_saleYear,total_numbersaleYear) == true)
        {   
            return true;
        }
        else 
        {
            Dbg_Println("Resources >> Save Total Yearly sale Error!");
            return false;  
        }
    }
    return false;
}
/*****************************************************************************/ 
int Resource_GetDaylySalesfromEerom (uint8_t date,uint8_t month,uint8_t year,int* numbersale)
{
    uint16_t failCount = 0;
    uint8_t crc_readRom = 0;
    Sale_t readSaleDayly;
    int saleReturn = 0;
    bool flag_getstart = false;
    uint32_t GetDaylySalespoint = 0;
    uint32_t Start_DaylySalespoint = 0;
    uint32_t Stop_DaylySalespoint = 0;

    *numbersale = 0;   
    Start_DaylySalespoint = calulator_poiterforgetsale(date,month,year);       
    if(Start_DaylySalespoint >= g_eepromsalepointer)
    {
        GetDaylySalespoint = EEPROM_SALE_START_DAY;
    }
    else 
    {
        if(g_eepromsalepointer - Start_DaylySalespoint < EEPROM_SALE_START_DAY)
        {
            GetDaylySalespoint = EEPROM_SALE_START_DAY;
        }
        else
        {
            GetDaylySalespoint = g_eepromsalepointer - Start_DaylySalespoint; 
        }
    }
    while(1)
    {
        if(EE_Read_Sale(GetDaylySalespoint,&readSaleDayly,10) == true)
        {
            crc_readRom = check_crc8bit(&readSaleDayly,9);
            if(crc_readRom == readSaleDayly.crc8bit)
            {
                if((date == readSaleDayly.Date)&&(month == readSaleDayly.Month)&&(year == readSaleDayly.Year))
                {
                    flag_getstart = true;
                    Stop_DaylySalespoint = GetDaylySalespoint + 100;
                    saleReturn += ((0x00FF&readSaleDayly.SaleValueH)<< 8)|readSaleDayly.SaleValueL;
                    *numbersale += (((0x00FF&readSaleDayly.numbersaleH)<< 8)|readSaleDayly.numbersaleL);
                }
                GetDaylySalespoint += 10;
                if((GetDaylySalespoint > g_eepromsalepointer)||((flag_getstart)&&(GetDaylySalespoint > Stop_DaylySalespoint)))
                {
                    return saleReturn*1000; 
                }
            }
            else 
            {
                GetDaylySalespoint += 10;
                failCount ++;
                if(failCount > 30)
                {
                    Dbg_Println("Resources >> Read Sale Dayly Error!");
                    return 0;
                }
            }
        }
        else 
        {
            failCount ++;
            if(failCount > 10)
            {
                Dbg_Println("Resources >> Read Sale Dayly Error!");
                return 0;
            }
        }
    }  
}
/*****************************************************************************/
int Resource_GetMonthlySalesfromEerom (uint8_t month,uint8_t year,int* numbersale)
{
    uint16_t failCount = 0;
    uint8_t crc_readRom = 0;
    Sale_t readSaleMonthly;
    int saleReturn = 0;
    uint32_t GetMonthlySalespoint = 0;
    uint32_t Start_MonthlySalespoint = 0;
    uint32_t Stop_MonthlySalespoint = 0;
    bool flag_getstart = false;
    
    *numbersale = 0;   
    Start_MonthlySalespoint = calulator_poiterforgetsale(1,month,year);        
    if(Start_MonthlySalespoint >= g_eepromsalepointer)
    {
        GetMonthlySalespoint = EEPROM_SALE_START_DAY;                           
    }
    else 
    {
        if(g_eepromsalepointer - Start_MonthlySalespoint < EEPROM_SALE_START_DAY)
        {
            GetMonthlySalespoint = EEPROM_SALE_START_DAY;
        }
        else
        {
            GetMonthlySalespoint = g_eepromsalepointer - Start_MonthlySalespoint;
        }
    }
    while(1)
    {
        if(EE_Read_Sale(GetMonthlySalespoint,&readSaleMonthly,10) == true)
        {
            crc_readRom = check_crc8bit(&readSaleMonthly,9);
            if(crc_readRom == readSaleMonthly.crc8bit)
            {
                if((readSaleMonthly.Date <= 31)&&(month == readSaleMonthly.Month)&&(year == readSaleMonthly.Year))
                {
                    flag_getstart = true;
                    Stop_MonthlySalespoint = GetMonthlySalespoint + 200;
                    saleReturn += ((0x00FF&readSaleMonthly.SaleValueH)<< 8)|readSaleMonthly.SaleValueL;
                    *numbersale += (((0x00FF&readSaleMonthly.numbersaleH)<< 8)|readSaleMonthly.numbersaleL);
                }
                GetMonthlySalespoint += 10;
                if((GetMonthlySalespoint > g_eepromsalepointer)||((flag_getstart)&&(GetMonthlySalespoint > Stop_MonthlySalespoint)))
                {
                    return saleReturn*1000; 
                }
            }
            else 
            {
                GetMonthlySalespoint += 10;
                failCount ++;
                if(failCount > 30)
                {
                    Dbg_Println("Resources >> Read Sale Mothly Error!");
                    return 0;
                }
            }
        }
        else 
        {
            failCount ++;
            if(failCount > 10)
            {
                Dbg_Println("Resources >> Read Sale Mothly Error!");
                return 0;
            }
        }
    }  
}
/*****************************************************************************/
int Resource_GetYearlySalesfromEerom (uint8_t year,int* numbersale)
{
    uint16_t failCount = 0;
    uint8_t crc_readRom = 0;
    SaleYear_t readSaleYearly;
    uint32_t GetYearlySalespoint;
    int saleReturn = 0;
    
    *numbersale = 0;  
    GetYearlySalespoint = EEPROM_SALE_START_YEAR;
    while(1)
    {
        if(EE_Read_Sale(GetYearlySalespoint,&readSaleYearly,13) == true)
        {
            crc_readRom = check_crc8bit(&readSaleYearly,12);
            if(crc_readRom == readSaleYearly.crc8bit)
            {
                if(year == readSaleYearly.Year)
                {
                    saleReturn += ((0x0000FFFF&readSaleYearly.SaleValueH)<< 16)|readSaleYearly.SaleValueL;
                    *numbersale += ((0x0000FFFF&readSaleYearly.numbersaleH)<< 16)|readSaleYearly.numbersaleL;
                }
                GetYearlySalespoint += 13;
                if(GetYearlySalespoint > EEPROM_SALE_STOP_YEAR)
                {
                    return saleReturn*1000;
                }
            }
            else 
            {
                GetYearlySalespoint += 13;
                if(GetYearlySalespoint > EEPROM_SALE_STOP_YEAR)
                {
                    return saleReturn*1000;
                }
            }
        }
        else 
        {
            failCount ++;
            if(failCount > 10)
            {
                Dbg_Println("Resources >> Read Sale Yearly Error!");
                return 0;
            }
        }
    }  
}
/*****************************************************************************/
int Resource_GetTotalSalesfromEerom (int* numbersale)
{
    uint16_t failCount = 0;
    uint8_t crc_readRom = 0;
    SaleYear_t readSaleYearly;
    uint32_t GetYearlySalespoint;
    int saleReturn = 0;
    
    *numbersale = 0;  
    GetYearlySalespoint = EEPROM_SALE_START_YEAR;
    while(1)
    {
        if(EE_Read_Sale(GetYearlySalespoint,&readSaleYearly,13) == true)
        {
            crc_readRom = check_crc8bit(&readSaleYearly,12);
            if(crc_readRom == readSaleYearly.crc8bit)
            {
                if(readSaleYearly.Year < 99)
                {
                    saleReturn += ((0x0000FFFF&readSaleYearly.SaleValueH)<< 16)|readSaleYearly.SaleValueL;
                    *numbersale += ((0x0000FFFF&readSaleYearly.numbersaleH)<< 16)|readSaleYearly.numbersaleL;
                }
                GetYearlySalespoint += 13;
                if(GetYearlySalespoint > EEPROM_SALE_STOP_YEAR)
                {
                    return saleReturn*1000;
                }
            }
            else 
            {
                GetYearlySalespoint += 13;
                if(GetYearlySalespoint > EEPROM_SALE_STOP_YEAR)
                {
                    return saleReturn*1000;
                }
            }
        }
        else 
        {
            failCount ++;
            if(failCount > 10)
            {
                Dbg_Println("Resources >> Read Total Sale Error!");
                return 0;
            }
        }
    }      
}
/*****************************************************************************/
bool SaveTotalDaylySaletoEerom (uint16_t sale, uint16_t number)
{
    uint16_t failCount = 0;
    uint8_t crc_writeRom = 0;
    uint8_t crc_readRom = 0;
    int date,month,year; 
    Sale_t Saledata;
    Sale_t CheckSalefromRom;
    Rtc_GetCurrentDate(&date, &month, &year); 
    Saledata.Header[0] = 0xFF;
    Saledata.Header[1] = 0xFF;
    Saledata.Date = date;
    Saledata.Month = month;
    Saledata.Year = year;
    Saledata.SaleValueH = (sale >> 8)&0x00FF;
    Saledata.SaleValueL = sale&0x00FF;
    Saledata.numbersaleH = (number >> 8)&0x00FF;
    Saledata.numbersaleL = number&0x00FF;
    crc_writeRom = check_crc8bit(&Saledata,9);
    Saledata.crc8bit = crc_writeRom;
    EE_Write_Sale(g_eepromsalepointer,&Saledata,10);
    while(1)
    {
        if(EE_Read_Sale(g_eepromsalepointer,&CheckSalefromRom,10) == true)
        {
            crc_readRom = check_crc8bit(&CheckSalefromRom,9);
            if(crc_readRom != crc_writeRom)
            {
                failCount++;
                if(failCount > 50)
                {
                    Dbg_Println("Resources >> Save Sale Dayly Error!");
                    return false;    
                }
                EE_Write_Sale(g_eepromsalepointer,&Saledata,10);
            }
            else return true;
        }
        else
        {
            failCount++;
            if(failCount > 50)
            {
                Dbg_Println("Resources >> Save Sale Dayly Error!");
                return false;    
            }
        }
    }  
}
/*****************************************************************************/
bool SaveTotalYearlySaletoEerom (uint32_t sale, uint32_t number)
{
    uint16_t failCount = 0;
    uint8_t crc_writeRom = 0;
    uint8_t crc_readRom = 0;
    int date,month,year; 
    SaleYear_t CheckSalefromRom;
    SaleYear_t SaleYeardata;
    
    Rtc_GetCurrentDate(&date, &month, &year); 
    SaleYeardata.Hearder = 0xFFFF;
    SaleYeardata.Year = 0x00FF&year;
    SaleYeardata.SaleValueH = (sale >> 16)&0x0000FFFF;
    SaleYeardata.SaleValueL = sale&0x0000FFFF;
    SaleYeardata.numbersaleH = (number >> 16)&0x0000FFFF;
    SaleYeardata.numbersaleL = number&0x0000FFFF;
    
    crc_writeRom = check_crc8bit(&SaleYeardata,12);
    SaleYeardata.crc8bit = crc_writeRom;
    EE_Write_Sale(g_eepromYearsalepointer,&SaleYeardata,13);
    while(1)
    {
        if(EE_Read_Sale(g_eepromYearsalepointer,&CheckSalefromRom,13) == true)
        {
            crc_readRom = check_crc8bit(&CheckSalefromRom,12);
            if(crc_readRom != crc_writeRom)
            {
                failCount++;
                if(failCount > 50)
                {
                    Dbg_Println("Resources >> Save Sale Yearly Error!");
                    return false;    
                }
                EE_Write_Sale(g_eepromYearsalepointer,&SaleYeardata,13);
            }
            else return true;
        }
        else
        {
            failCount++;
            if(failCount > 50)
            {
                Dbg_Println("Resources >> Save Sale Yearly Error!");
                return false;    
            }
        }
    }   
}
/*****************************************************************************/
void SaveEepromsalepointer (void)
{
    uint16_t tmp; 
    uint16_t failCount = 0;
    
    uint8_t* p = (uint8_t*)(&g_eepromsalepointer);
    EE_Write_Sale(EEPROM_SALE_POINTER, p, 2);
    while(1)
    {
        tmp = 0;
        EE_Read_Sale(EEPROM_SALE_POINTER, &tmp, 2);
        if(tmp != g_eepromsalepointer)
        {
            failCount++;
            if(failCount > 50)
            {
                Dbg_Println("Resources >> Save Sale Pointer Error!");
                break;    
            }
            EE_Write_Sale(EEPROM_SALE_POINTER, p, 2);   
        }
        else break;
    }
}
/*****************************************************************************/
void ReadEepromsalepointer (void)
{
    uint16_t tmp[10];
    uint16_t failCount = 0;
    uint8_t i = 0;
    while(1)
    {
        for(i = 0; i < 10; i++)
        {
            tmp[i] = 0;
        }
        for(i = 0; i < 10; i++)
        {
            EE_Read_Sale(EEPROM_SALE_POINTER, &tmp[i], 2);
            Delay(50);
            if(i > 0)
            {
                if(tmp[i] != tmp[i - 1])
                {
                    failCount++;
                    if(failCount > 10)
                    {
                        Dbg_Println("Resources >> Read Sale Pointer Error!");
                        break; 
                    }
                }
            }
        }
        if(failCount > 10)
        {
            g_eepromsalepointer = EEPROM_SALE_START_DAY;
            return;
        }
        else 
        {
            g_eepromsalepointer = tmp[0];
            if((g_eepromsalepointer == 0xFFFF)||
              (g_eepromsalepointer < EEPROM_SALE_START_DAY)||
              (g_eepromsalepointer > EEPROM_SALE_STOP_DAY)) 
            {
                g_eepromsalepointer = EEPROM_SALE_START_DAY;
                return;
            }
            else 
            {
                break;
            }
        }   
    }
}
/*****************************************************************************/
void SaveEepromYearsalepointer (void)
{
    uint16_t tmp; 
    uint16_t failCount = 0;
    
    uint8_t* p = (uint8_t*)(&g_eepromYearsalepointer);
    EE_Write_Sale(EEPROM_SALE_YEAR_POINTER, p, 2);
    while(1)
    {
        tmp = 0;
        EE_Read_Sale(EEPROM_SALE_YEAR_POINTER, &tmp, 2);
        if(tmp != g_eepromYearsalepointer)
        {
            failCount++;
            if(failCount > 50)
            {
                Dbg_Println("Resources >> Save Year Sale Pointer Error!");
                break;    
            }
            EE_Write_Sale(EEPROM_SALE_YEAR_POINTER, p, 2);   
        }
        else break;
    }
}
/*****************************************************************************/
void ReadEepromYearsalepointer (void)
{
    uint16_t tmp[10];
    uint16_t failCount = 0;
    uint8_t i = 0;
    while(1)
    {
        for(i = 0; i < 10; i++)
        {
            tmp[i] = 0;
        }
        for(i = 0; i < 10; i++)
        {
            EE_Read_Sale(EEPROM_SALE_YEAR_POINTER, &tmp[i], 2);
            Delay(50);
            if(i > 0)
            {
                if(tmp[i] != tmp[i - 1])
                {
                    failCount++;
                    if(failCount > 10)
                    {
                        Dbg_Println("Resources >> Read Year Sale Pointer Error!");
                        break; 
                    }
                }
            }
        }
        if(failCount > 10)
        {
            g_eepromYearsalepointer = EEPROM_SALE_START_YEAR;
            return;
        }
        else 
        {
            g_eepromYearsalepointer = tmp[0];
            if((g_eepromYearsalepointer == 0xFFFF)||
              (g_eepromYearsalepointer < EEPROM_SALE_START_YEAR)||
              (g_eepromYearsalepointer > EEPROM_SALE_STOP_YEAR)) 
            {
                g_eepromYearsalepointer = EEPROM_SALE_START_YEAR;
                return;
            }
            else 
            {
                break;
            }
        }   
    }
}
/*****************************************************************************/
uint32_t calulator_poiterforgetsale (uint8_t checkdate,uint8_t checkmonth,uint8_t checkyear)
{
    int date,month,year; 
    uint32_t calculator_pointer = 0;
    Rtc_GetCurrentDate(&date, &month, &year); 
    if(checkyear <= year)
    {
        if(checkyear == year)
        {
            if(checkmonth <= month)
            {
                if(checkmonth == month)
                {
                    if(checkdate <= date)
                    {
                        calculator_pointer = ((date - checkdate) + 10)*10; //bu them 10 ngay
                    }
                    else return 0; 
                }
                else
                {
                    if(checkdate <= date)
                    {
                        calculator_pointer = ((((month - checkmonth)*31) + (date - checkdate)) + 10)*10;  //bu them 10 ngay
                    }
                    else
                    {
                        calculator_pointer = ((((month - checkmonth)*31) - (checkdate - date)) + 10)*10;  //bu them 10 ngay
                    }
                }
            }
            else return 0;
        }
        else
        {
            if(checkmonth <= month)
            {
                if(checkdate <= date)
                {
                    calculator_pointer = ((((year - checkyear)*365) + ((month - checkmonth)*31) + (date - checkdate)) + 10)*10;     //bu them 10 ngay
                }
                else 
                {
                    calculator_pointer = ((((year - checkyear)*365) + ((month - checkmonth)*31) - (checkdate - date)) + 10)*10;     //bu them 10 ngay         
                }
            }
            else 
            {
                if(checkdate <= date)
                {
                    calculator_pointer = ((((year - checkyear)*365) - ((checkmonth - month)*31) + (date - checkdate)) + 10)*10;     //bu them 10 ngay  
                }
                else
                {
                    calculator_pointer = ((((year - checkyear)*365) - ((checkmonth - month)*31) - (checkdate - date)) + 10)*10;     //bu them 10 ngay    
                }
            }
        }
    }
    else return 0;
    return (calculator_pointer);
}

/*****************************************************************************/
void Resource_ClearSaleDay(void)
{
  /* init variable */
  uint8_t bufData = 0;
  uint8_t status = 0;
  /* Doc vi tri con tro luu tru sale Day trong EEprom*/
  ReadEepromsalepointer();
  /* Clear sale day */
  for(uint32_t address = EEPROM_SALE_START_DAY ; address <= g_eepromsalepointer; address++)
  {
    EE_WriteByte_Sale(address, 0x00);
    status = 0;
    /* Verity data */
    while(1)
    {
      if(EE_ReadByte_Sale(address, &bufData) == false || bufData != 0x00)
      {
        EE_WriteByte_Sale(address, 0x00);
        status++;
        if(status == 5) break;
      }
      else
      {
        break;
      }
    } /* <end while >*/
  } /* <end for >*/
  
  /* Clear EEProm sale pointer */
  g_eepromsalepointer = EEPROM_SALE_START_DAY;
  SaveEepromsalepointer();
}



/*****************************************************************************/
void Resource_ClearSaleYear(void)
{
  /* init variable */
  uint8_t bufData = 0;
  uint8_t status = 0;
  /* Doc vi tri con tro luu tru sale year trong EEprom*/
  ReadEepromYearsalepointer();
  /* Clear sale day */
  for(uint32_t address = EEPROM_SALE_START_YEAR ; address <= g_eepromYearsalepointer; address++)
  {
    EE_WriteByte_Sale(address, 0x00);
    status = 0;
    /* Verity data */
    while(1)
    {
      if(EE_ReadByte_Sale(address, &bufData) == false || bufData != 0x00)
      {
        EE_WriteByte_Sale(address, 0x00);
        status++;
        if(status == 5) break;
      }
      else
      {
        break;
      }
    } /* <end while >*/
  } /* <end for >*/
  
  /* Clear EEProm sale pointer */
  g_eepromYearsalepointer = EEPROM_SALE_START_YEAR;
  SaveEepromYearsalepointer();
}
