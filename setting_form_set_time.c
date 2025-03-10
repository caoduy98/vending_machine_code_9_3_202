/** @file    setting_form_set_time.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat thoi gian
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"

/*********************************************************************************
 * DEFINE
 */

#define NO_CONTROL

#define TIME_SETTING_OFFSET_X     35
#define TIME_SETTING_OFFSET_Y     15

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void DrawPointer(int x, int y);
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
#ifdef CONTROL
static bool g_pointerIsOn = true;
static int8_t g_selectedIndex = 0;
#endif
static bool g_visible = false;
static timer_t g_timer;
static int g_hour = 0, g_minute = 0, g_second = 0, g_date = 0, g_month = 0, g_year = 2018;

/*********************************************************************************
 * STATIC FUNCTION
 */

#ifdef CONTROL
static void TimerHandler(timer_t* tmr, void* param);

/**
 * @brief    Cap nhat gia tri hien thi gio tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void UpdateHour()
{
    GLcd_FillRect(TIME_SETTING_OFFSET_X, TIME_SETTING_OFFSET_Y, 14, 9, BLACK);
    
    if (g_pointerIsOn)
    {
        GLcd_SetFont(&font7x9);
        sprintf(strLcd, "%02d", g_hour);
        GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X, TIME_SETTING_OFFSET_Y, WHITE);
    }
    
    GLcd_FlushRegion(TIME_SETTING_OFFSET_Y, 9);
}

/**
 * @brief    Cap nhat gia tri hien thi phut tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void UpdateMinute()
{
    
    GLcd_FillRect(TIME_SETTING_OFFSET_X + 19, TIME_SETTING_OFFSET_Y, 14, 9, BLACK);
    
    if (g_pointerIsOn)
    {
        GLcd_SetFont(&font7x9);
        sprintf(strLcd, "%02d", g_minute);
        GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 19, TIME_SETTING_OFFSET_Y, WHITE);
    }
    
    GLcd_FlushRegion(TIME_SETTING_OFFSET_Y, 9);
}

/**
 * @brief    Cap nhat gia tri hien thi giay tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void UpdateSecond()
{
    GLcd_FillRect(TIME_SETTING_OFFSET_X + 38, TIME_SETTING_OFFSET_Y, 14, 9, BLACK);
    
    if (g_pointerIsOn)
    {
        GLcd_SetFont(&font7x9);
        sprintf(strLcd, "%02d", g_second);
        GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 38, TIME_SETTING_OFFSET_Y, WHITE);
    }
    
    GLcd_FlushRegion(TIME_SETTING_OFFSET_Y, 9);
}

/**
 * @brief    Cap nhat gia tri hien thi ngay tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void UpdateDate()
{
    GLcd_FillRect(TIME_SETTING_OFFSET_X, TIME_SETTING_OFFSET_Y + 20, 14, 9, BLACK);
    
    if (g_pointerIsOn)
    {
        GLcd_SetFont(&font7x9);
        sprintf(strLcd, "%02d", g_date);
        GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X, TIME_SETTING_OFFSET_Y + 20, WHITE);
    }
    
    GLcd_FlushRegion(TIME_SETTING_OFFSET_Y + 20, 9);
}

/**
 * @brief    Cap nhat gia tri hien thi thang tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void UpdateMonth()
{
    GLcd_FillRect(TIME_SETTING_OFFSET_X + 22, TIME_SETTING_OFFSET_Y + 20, 14, 9, BLACK);
    
    if (g_pointerIsOn)
    {
        GLcd_SetFont(&font7x9);
        sprintf(strLcd, "%02d", g_month);
        GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 22, TIME_SETTING_OFFSET_Y + 20, WHITE);
    }
    
    GLcd_FlushRegion(TIME_SETTING_OFFSET_Y + 20, 9);
}

/**
 * @brief    Cap nhat gia tri hien thi nam tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void UpdateYear()
{
    GLcd_FillRect(TIME_SETTING_OFFSET_X + 44, TIME_SETTING_OFFSET_Y + 20, 28, 9, BLACK);
    
    if (g_pointerIsOn)
    {
        GLcd_SetFont(&font7x9);
        sprintf(strLcd, "%04d", g_year);
        GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 44, TIME_SETTING_OFFSET_Y + 20, WHITE);
    }
    
    GLcd_FlushRegion(TIME_SETTING_OFFSET_Y + 20, 9);
}
#endif

/**
 * @brief    Hient thi toan bo menu cai dat thoi gian len LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* ve background */
    GLcd_ClearScreen(BLACK);
    
    GLcd_SetFont(&font6x8);
    GLcd_DrawString("TIME:", TIME_SETTING_OFFSET_X - 35, TIME_SETTING_OFFSET_Y + 1, WHITE);
    GLcd_DrawString("DATE:", TIME_SETTING_OFFSET_X - 35, TIME_SETTING_OFFSET_Y + 21, WHITE);
    
    /* HOUR:MINUTE:SECOND */
    GLcd_SetFont(&font7x9);
    sprintf(strLcd, "%02d", g_hour);
    GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X, TIME_SETTING_OFFSET_Y, WHITE);
    GLcd_SetFont(&font6x8);
    GLcd_DrawString(":", TIME_SETTING_OFFSET_X + 14, TIME_SETTING_OFFSET_Y, WHITE);
    
    GLcd_SetFont(&font7x9);
    sprintf(strLcd, "%02d", g_minute);
    GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 19, TIME_SETTING_OFFSET_Y, WHITE);
    GLcd_SetFont(&font6x8);
    GLcd_DrawString(":", TIME_SETTING_OFFSET_X + 33, TIME_SETTING_OFFSET_Y, WHITE);
    
    GLcd_SetFont(&font7x9);
    sprintf(strLcd, "%02d", g_second);
    GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 38, TIME_SETTING_OFFSET_Y, WHITE);
    
    /* DATE-MONTH-YEAR */
    GLcd_SetFont(&font7x9);
    sprintf(strLcd, "%02d", g_date);
    GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X, TIME_SETTING_OFFSET_Y + 20, WHITE);
    GLcd_SetFont(&font6x8);
    GLcd_DrawString("-", TIME_SETTING_OFFSET_X + 15, TIME_SETTING_OFFSET_Y + 20, WHITE);
    
    GLcd_SetFont(&font7x9);
    sprintf(strLcd, "%02d", g_month);
    GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 22, TIME_SETTING_OFFSET_Y + 20, WHITE);
    GLcd_SetFont(&font6x8);
    GLcd_DrawString("-", TIME_SETTING_OFFSET_X + 37, TIME_SETTING_OFFSET_Y + 20, WHITE);
    
    GLcd_SetFont(&font7x9);
    sprintf(strLcd, "%04d", g_year);
    GLcd_DrawString(strLcd, TIME_SETTING_OFFSET_X + 44, TIME_SETTING_OFFSET_Y + 20, WHITE);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}


/*********************************************************************************
 * EXPORTED FUNCTION
 */


/**
 * @brief    Hien thi menu cai dat thoi gian len tren LCD
 * 
 * @param    uint32_t id : Not using
 * @retval   NONE
 */
void SettingSetTime_Show(uint32_t id)
{
    /* Initialization variable */
    g_visible = true;
    #ifdef CONTROL
    g_pointerIsOn = true;
    g_selectedIndex = 0;
    #endif
    
    /* lay thoi gian thuc tu DS1307 */
    Rtc_GetCurrentTime(&g_hour, &g_minute, &g_second);
    Rtc_GetCurrentDate(&g_date, &g_month, &g_year);
    g_year += 2000;
    /* Hien thi menu cai dat thoi gian  */
    Paint();
    /* bat dau soft time de Blink chu so */
    #ifdef CONTROL
    Tmr_Start(&g_timer, 400, TimerHandler, NULL);
    #endif
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat thoi gian
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel     
 */
bool SettingSetTime_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim cancel thi tra ve false  va hien thi menu 
       va dung soft time */
    if (keyCode == '#')    /* CANCEL KEY */
    {
        g_visible = false;
        Tmr_Stop(&g_timer);
        /* hien thi menu */
        Setting_Show();
    }
    #ifdef CONTROL
    else if (keyCode == '*')    /* ENTER KEY */
    {
        g_selectedIndex++;
        if (g_selectedIndex > 5) /* Neu nhan ENTER lan thu 6 */
        {
            g_selectedIndex = 0;
            /* Luu thoi gian cai dat vao DS1307 */
            Rtc_SetTime(g_hour, g_minute, g_second);
            Rtc_SetDate(g_date, g_month, g_year);
            /* Ve chu "DONE" len LCD va du hien thi trong 1500 ms*/
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
            Delay(1500);
        }
        /* Hien thi menu cai dat thoi gian */
        Paint();
    }
    else if (keyCode == '0')    /* DOWN ARROW */
    {
        /* Reset soft time */
        Tmr_Reset(&g_timer);
        g_pointerIsOn = true;
        /* thay doi thoi gian cai dat */
        switch (g_selectedIndex)
        {
        case 0:
            g_hour--;
            if (g_hour < 0)
            {
                g_hour = 23;
            }
            UpdateHour();
            break;
        case 1:
            g_minute--;
            if (g_minute < 0)
            {
                g_minute = 59;
            }
            UpdateMinute();
            break;
        case 2:
            g_second--;
            if (g_second < 0)
            {
                g_second = 59;
            }
            UpdateSecond();
            break;
        case 3:
            g_date--;
            if (g_date < 0)
            {
                g_date = 31;
            }
            UpdateDate();
            break;
        case 4:
            g_month--;
            if (g_month < 0)
            {
                g_month = 12;
            }
            UpdateMonth();
            break;
        case 5:
            if (g_year > 3000)
            {
                g_year = 3000;
            }
            g_year--;
            if (g_year < 0)
            {
                g_year = 2050;
            }
            UpdateYear();
            break;
        }
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        /* Reset soft time */
        Tmr_Reset(&g_timer);
        g_pointerIsOn = true;
        /* thay doi thoi gian cai dat */
        switch (g_selectedIndex)
        {
        case 0:
            g_hour++;
            if (g_hour > 23)
            {
                g_hour = 0;
            }
            UpdateHour();
            break;
        case 1:
            g_minute++;
            if (g_minute > 59)
            {
                g_minute = 0;
            }
            UpdateMinute();
            break;
        case 2:
            g_second++;
            if (g_second > 59)
            {
                g_second = 0;
            }
            UpdateSecond();
            break;
        case 3:
            g_date++;
            if (g_date > 31)
            {
                g_date = 0;
            }
            UpdateDate();
            break;
        case 4:
            g_month++;
            if (g_month > 12)
            {
                g_month = 0;
            }
            UpdateMonth();
            break;
        case 5:
            if (g_year < 2000)
            {
                g_year = 2000;
            }
            g_year++;
            if (g_year > 2050)
            {
                g_year = 0;
            }
            UpdateYear();
            break;
        }
    }
    #endif
    return true;
}


/**
 * @brief    Ham xu ly du lieu khi co phim giu luc LCD dang hien thi menu cai dat thoi gian
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool        
 */

bool SettingSetTime_KeyHold(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    #ifdef CONTROL
    if (keyCode == '0')    /* DOWN ARROW */
    {
        /* Reset soft time */
        Tmr_Reset(&g_timer);
        g_pointerIsOn = true;
        /* thay doi thoi gian cai dat */
        switch (g_selectedIndex)
        {
        case 0:
            g_hour--;
            if (g_hour < 0)
            {
                g_hour = 23;
            }
            UpdateHour();
            break;
        case 1:
            g_minute--;
            if (g_minute < 0)
            {
                g_minute = 59;
            }
            UpdateMinute();
            break;
        case 2:
            g_second--;
            if (g_second < 0)
            {
                g_second = 59;
            }
            UpdateSecond();
            break;
        case 3:
            g_date--;
            if (g_date < 0)
            {
                g_date = 31;
            }
            UpdateDate();
            break;
        case 4:
            g_month--;
            if (g_month < 0)
            {
                g_month = 12;
            }
            UpdateMonth();
            break;
        case 5:
            g_year--;
            if (g_year < 0)
            {
                g_year = 2050;
            }
            UpdateYear();
            break;
        }
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        /* Reset soft time */
        Tmr_Reset(&g_timer);
        g_pointerIsOn = true;
        /* thay doi thoi gian cai dat */
        switch (g_selectedIndex)
        {
        case 0:
            g_hour++;
            if (g_hour > 23)
            {
                g_hour = 0;
            }
            UpdateHour();
            break;
        case 1:
            g_minute++;
            if (g_minute > 59)
            {
                g_minute = 0;
            }
            UpdateMinute();
            break;
        case 2:
            g_second++;
            if (g_second > 59)
            {
                g_second = 0;
            }
            UpdateSecond();
            break;
        case 3:
            g_date++;
            if (g_date > 31)
            {
                g_date = 0;
            }
            UpdateDate();
            break;
        case 4:
            g_month++;
            if (g_month > 12)
            {
                g_month = 0;
            }
            UpdateMonth();
            break;
        case 5:
            g_year++;
            if (g_year > 2050)
            {
                g_year = 0;
            }
            UpdateYear();
            break;
        }
    }
    #endif
    return true;
}

/**
 * @brief    Hient thi toan bo menu cai dat thoi gian len LCD
 * 
 * @param    timer_t*      con tro toi software timer
 * @param    void* param   parameter send to function callback
 * @retval   NONE
 */
#ifdef CONTROL
static void TimerHandler(timer_t* tmr, void* param)
{
    if (g_pointerIsOn == true)
    {
        g_pointerIsOn = false;
        Tmr_Start(&g_timer, 700, TimerHandler, NULL);
    }
    else
    {
        g_pointerIsOn = true;
        Tmr_Start(&g_timer, 400, TimerHandler, NULL);
    }
    
    switch (g_selectedIndex)
    {
    case 0:
        UpdateHour();
        break;
    case 1:
        UpdateMinute();
        break;
    case 2:
        UpdateSecond();
        break;
    case 3:
        UpdateDate();
        break;
    case 4:
        UpdateMonth();
        break;
    case 5:
        UpdateYear();
        break;
    }
}
#endif

/**
 * @brief    Thoat menu cai dat thoi gian
 * 
 * @param    NONE
 * @retval   bool  : trang thai hien thi menu cai dat thoi gian
 */
bool OutSettingTime()
{
     g_visible = false;
     #ifdef CONTROL
     Tmr_Stop(&g_timer);
     #endif
     return g_visible;
}
