#ifndef RTC_H
#define RTC_H

bool DS1307_Init();
bool DS1307_Read(uint8_t address, uint8_t* data);
bool DS1307_Write(uint8_t address, uint8_t data);
uint8_t DS1307_GetSecond();

bool Rtc_GetCurrentTime(int* hour, int* minute, int* second);
bool Rtc_GetCurrentDate(int* date, int* month, int* year);
bool Rtc_SetTime(int hour, int minute, int second);
bool Rtc_SetDate(int date, int month, int year);
int Rtc_CompareDate(int date1, int month1, int year1, int date2, int month2, int year2);
bool write_time_process (int date,int month,int year,int hour,int minute,int second);

#endif