
#include "platform.h"
#include "rtc.h"
#include "swi2c.h"

#define DS1307_SLA_W  0XD0
#define DS1307_SLA_R  0XD1

typedef struct
{
    int date;
    int month;
    int year;
    int hour;
    int minute;
    int second;
} rtc_time_t;

extern swi2c_t g_softI2C0;

/**
 * @brief  Ham khoi tao cho RTC
 * 
 * @param  NONE                
 * @retval NONE     
 */
bool DS1307_Init()
{
    //Clear CH bit of RTC
    #define CH 7

    uint8_t temp;
    if (!DS1307_Read(0x00, &temp))    return false;

    //Clear CH Bit
    temp &= (~(1 << CH));

    if (!DS1307_Write(0x00, temp))   return false;

    if (!DS1307_Read(0x02, &temp))   return false;

    //Set 24Hour BIT
    temp &= ~0x40;

    //Write Back to DS1307
    if (!DS1307_Write(0x02, temp))   return false;

    return true;
}

/**
 * @brief  Ham doc du lieu thoi gian tu RTC
 * 
 * @param  uint8_t address -> dia chi bat dau
 * @retval uint8_t* data -> Mang chua du lieu thoi gian tra ve tu RTC            
 * @retval bool DS1307_Read -> Trang thai doc du lieu     
 */
bool DS1307_Read(uint8_t address, uint8_t* data)
{
   uint8_t res;   //result

   //Start

   SWI2C_Start(&g_softI2C0);

   //SLA+W (for dummy write to set register pointer)
   res = SWI2C_WriteByte(&g_softI2C0, DS1307_SLA_W); //DS1307 address + W

   //Error
   if (!res) return false;

   //Now send the address of required register

   res = SWI2C_WriteByte(&g_softI2C0, address);

   //Error
   if (!res) return false;

   //Repeat Start
   SWI2C_Start(&g_softI2C0);

   //SLA + R
   res = SWI2C_WriteByte(&g_softI2C0, DS1307_SLA_R); //DS1307 Address + R

   //Error
   if (!res) return false;

   //Now read the value with NACK
   *data = SWI2C_ReadByte(&g_softI2C0, 0);

   //Error

   if (!res) return false;

   //STOP
   SWI2C_Stop(&g_softI2C0);

   return true;
}
/**
 * @brief  Ham ghi du lieu vao RTC
 * 
 * @param  uint8_t address -> Dia chi can ghi 
 * @param  uint8_t data -> Du lieu can ghi          
 * @retval bool DS1307_Read -> Trang thai ghi du lieu     
 */
bool DS1307_Write(uint8_t address, uint8_t data)
{
   uint8_t res;   //result

   //Start
   SWI2C_Start(&g_softI2C0);

   //SLA+W
   res = SWI2C_WriteByte(&g_softI2C0, DS1307_SLA_W); //DS1307 address + W

   //Error
   if (!res) return false;

   //Now send the address of required register
   res = SWI2C_WriteByte(&g_softI2C0, address);

   //Error
   if (!res) return false;

   //Now write the value

   res = SWI2C_WriteByte(&g_softI2C0, data);

   //Error
   if (!res) return false;

   //STOP
   SWI2C_Stop(&g_softI2C0);

   return true;
}

/**
 * @brief  Ham doc du lieu thoi gian (giay) tu RTC
 * 
 * @param  NONE      
 * @retval uint8_t DS1307_GetSecond -> Du lieu thoi gian (giay) doc duoc  
 */
uint8_t DS1307_GetSecond()
{
    uint8_t sec,temp;

    //Read the Second Register
    DS1307_Read(0x00, &temp);
//    sec = (((temp & 0b01110000) >> 4) * 10) + (temp & 0b00001111);
    sec = (((temp & 0x70) >> 4) * 10) + (temp & 0x0F);

    return sec;


}
/**
 * @brief  Ham doc du lieu thoi gian (gio, phut, giay) hien tai tu RTC
 * 
 * @param  NONE      
 * @retval int* hour -> Du lieu thoi gian (gio) doc duoc
 * @retval int* minute -> Du lieu thoi gian (phut) doc duoc
 * @retval int* second -> Du lieu thoi gian (giay) doc duoc
 * @retval bool Rtc_GetCurrentTime -> Trang thai doc du lieu  
 */
bool Rtc_GetCurrentTime(int* hour, int* minute, int* second)
{
    uint8_t ss, mm, hh;
    
    if (DS1307_Read(0x00, &ss) == false)
    {
        return false;
    }
    if (DS1307_Read(0x01, &mm) == false)
    {
        return false;
    }
    if (DS1307_Read(0x02, &hh) == false)
    {
                return false;
    }
    
    *second = (((ss & 0x70) >> 4) * 10) + (ss & 0x0F);
    *minute = (((mm & 0x70) >> 4) * 10) + (mm & 0x0F);
    *hour = (((hh & 0x30) >> 4) * 10) + (hh & 0x0F);
    
    return true;
}
/**
 * @brief  Ham doc du lieu ngay thang (ngay, thang, nam) hien tai tu RTC
 * 
 * @param  NONE      
 * @retval int* date -> Du lieu ngay thang (ngay) doc duoc
 * @retval int* month -> Du lieu ngay thang (thang) doc duoc
 * @retval int* year -> Du lieu ngay thang (nam) doc duoc 
 * @retval bool Rtc_GetCurrentDate -> Trang thai doc du lieu
 */
bool Rtc_GetCurrentDate(int* date, int* month, int* year)
{
    uint8_t DD, MM, YY;
    
    if (DS1307_Read(0x04, &DD) == false)
    {
        return false;
    }
    if (DS1307_Read(0x05, &MM) == false)
    {
        return false;
    }
    if (DS1307_Read(0x06, &YY) == false)
    {
        return false;
    }
    
    *date = (((DD & 0x30) >> 4) * 10) + (DD & 0x0F);
    *month = (((MM & 0x10) >> 4) * 10) + (MM & 0x0F);
    *year = (((YY & 0xF0) >> 4) * 10) + (YY & 0x0F);
    
    return true;
}
/**
 * @brief  Ham ghi du lieu thoi gian (gio, phut, giay) toi RTC
 * 
 * @param  int hour -> Du lieu thoi gian (gio) cai dat      
 * @param  int minute -> Du lieu thoi gian (phut) cai dat 
 * @param  int second -> Du lieu thoi gian (giay) cai dat 
 * @retval bool Rtc_SetTime -> Trang thai ghi du lieu  
 */
bool Rtc_SetTime(int hour, int minute, int second)
{
    if (hour < 1)   hour = 0;
    if (hour > 23)  hour = 23;
    if (minute < 1)   minute = 0;
    if (minute > 59)  minute = 59;
    if (second < 1)   second = 0;
    if (second > 59)  second = 59;
    
    uint8_t temp;
    temp = ((second / 10) << 4) | (second % 10);
    if (DS1307_Write(0x00, temp) == false)
    {
        return false;
    }
    Delay(5);
    temp = ((minute / 10) << 4) | (minute % 10);
	if (DS1307_Write(0x01, temp) == false)
    {
        return false;
    }
    Delay(5);
    temp = ((hour / 10) << 4) | (hour % 10);
	temp &= ~0x40; //24 Hr Mode
    if (DS1307_Write(0x02, temp) == false)
    {
        return false;
    }
    Delay(5);
    return true;
}
/**
 * @brief  Ham ghi du lieu ngay thang (ngay, thang, nan) toi RTC
 * 
 * @param  int date -> Du lieu ngay thang (ngay) cai dat      
 * @param  int month -> Du lieu ngay thang (thang) cai dat 
 * @param  int year -> Du lieu ngay thang (nam) cai dat 
 * @retval bool Rtc_SetTime -> Trang thai ghi du lieu  
 */
bool Rtc_SetDate(int date, int month, int year)
{
    if (date < 1)   date = 1;
    if (date > 31)  date = 31;
    if (month < 1)   month = 1;
    if (month > 12)  month = 12;
    if (year < 2000)   year = 2000;
    if (year > 2099)  year = 2099;
    
    uint8_t temp;
    temp = ((date / 10) << 4) | (date % 10);
    if (DS1307_Write(0x04, temp) == false)
    {
        return false;
    }
    Delay(5);
    temp = ((month / 10) << 4) | (month % 10);
	if (DS1307_Write(0x05, temp) == false)
    {
        return false;
    }
    Delay(5);
    year -= 2000;
    temp = ((year / 10) << 4) | (year % 10);
    if (DS1307_Write(0x06, temp) == false)
    {
        return false;
    }
    Delay(5);
    return true;
}
bool write_time_process (int date,int month,int year,int hour,int minute,int second)
{
    uint8_t fail_count = 0;
    rtc_time_t set_time;
    rtc_time_t read_rtc;
    
    set_time.date = date;
    set_time.month = month;
    set_time.year = year;
    set_time.hour = hour;
    set_time.minute = minute;
    set_time.second = second;
    Rtc_SetDate(set_time.date,set_time.month,set_time.year);
    Delay(5);
    Rtc_SetTime(set_time.hour,set_time.minute,set_time.second);
    while (1)
    {
        Rtc_GetCurrentDate(&read_rtc.date,&read_rtc.month,&read_rtc.year);
        Rtc_GetCurrentTime(&read_rtc.hour,&read_rtc.minute,&read_rtc.second);
        if((set_time.date == read_rtc.date)&&(set_time.month == read_rtc.month)&&
          (set_time.year == read_rtc.year + 2000)&&(set_time.hour == read_rtc.hour)&&
          (set_time.minute == read_rtc.minute))
        {
            return true;
        }
        else
        {
            fail_count++;
            if(fail_count > 20)
            {
                return false;
            }
            Rtc_SetDate(set_time.date,set_time.month,set_time.year);
            Delay(50);
            Rtc_SetTime(set_time.hour,set_time.minute,set_time.second);
        }

        /* Check tick all time */
        Perh_ProcessAllWhile();
    } 
}
/**
 * @brief  Ham ghi so sanh du lieu ngay thang 
 */
int Rtc_CompareDate(int date1, int month1, int year1, int date2, int month2, int year2)
{
    if (year1 > year2)
    {
        return 1;
    }
    else if (year1 < year2)
    {
        return -1;
    }
    else
    {
        if (month1 > month2)
        {
            return 1;
        }
        else if (month1 < month2)
        {
            return -1;
        }
        else
        {
            if (date1 > date2)
            {
                return 1;
            }
            else if (date1 < date2)
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }
    }
}
