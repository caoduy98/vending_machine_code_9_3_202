/** @file    setting_form_onoff.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat dia chi Server
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "language.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_ONOFF
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void DrawPointer(int x, int y);
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
static int8_t g_selectedIndex = 0;
static bool g_visible = false;
static uint32_t g_combineId = 0;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi trang thai bat tat tren LCD
 *
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* ve background */
    GLcd_ClearScreen(BLACK);
    DrawTopGuideLine();
    /* ve title */
    GLcd_SetFont(&font6x8);
    GLcd_DrawStringUni(Lang_GetText(LANG_ON), 20, 20, WHITE);
    GLcd_DrawStringUni(Lang_GetText(LANG_OFF), 20, 35, WHITE);
    /* ve pointer */
    DrawPointer(5, 26 + g_selectedIndex * 15);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu cai dat bat tat
 *
 * @param    const uint16_t* title  - con tro den vi tri text hien thi
 * @param    uint32_t id            -   lua chon hien thi menu
 *
 * @retval   NONE
 */
void SettingOnOff_Show(const uint16_t* title, uint32_t id)
{
    /* Initialization variable */
    g_visible = true;
    g_selectedIndex = 0;
    g_combineId = id;
    /* neu hien thi menu ON/OFF do am */
    if (g_combineId == ID_HUMIDITY_ONOFF)
    {
        if (!Perh_HeaterIsOn())
        {
            g_selectedIndex = 1;
        }
    }
    /* neu hien thi menu ON/OFF cam bien roi */
    else if (g_combineId == ID_DROP_SENSOR)
    {
        if (!Perh_DropSensorIsOn())
        {
            g_selectedIndex = 1;
        }
    }
    /* neu hien thi menu ON/OFF AUDIO */
    else if (g_combineId == ID_AUDIO_SETTING)
    {
        if (!Perh_AudioIsOn())
        {
            g_selectedIndex = 1;
        }
    }
    /* neu hien thi menu ON/OFF tien tra lai */
    else if (g_combineId == ID_BILL_CHANGE)
    {
        if (!Perh_RefundIsOn())
        {
            g_selectedIndex = 1;
        }
    }
    /* Hien thi menu cai dat ON/OFF */
    Paint();
}


/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat ON/OFF
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingOnOff_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim mui ten xuong */
    if (keyCode == '0')
    {
        g_selectedIndex++;
        if (g_selectedIndex > 1)
        {
            g_selectedIndex = 0;
        }
        /* Hien thi menu cai dat ON/OFF */
        Paint();
    }
    /* neu nhan phim mui ten len */
    else if (keyCode == '8')
    {
        g_selectedIndex--;
        if (g_selectedIndex < 0)
        {
            g_selectedIndex = 1;
        }
        /* Hien thi menu cai dat ON/OFF */
        Paint();
    }
    /* neu nhan phim cancel thi tra ve false  va hien thi menu */
    else if (keyCode == '#')    /* CANCEL KEY */
    {
        g_visible = false;
        /* hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {
        /* neu dang cai dat ON/OFF led */
        if (g_combineId == ID_HUMIDITY_ONOFF)
        {
            if (g_selectedIndex == 0)
            {
                Perh_HeaterOn();
                Dbg_Println("Setting >> Bat chuong trinh dieu khien heater");
            }
            else
            {
                Perh_HeaterOff();
                Dbg_Println("Setting >> Tat den led");
            }
            /* hien thi "DONE" tren man hinh trong 1500ms */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Hien thi menu cai dat do am */
            Paint();
        }
        /* neu dang cai dat ON/OFF cam bien do am */
        else if (g_combineId == ID_DROP_SENSOR)
        {
            if (g_selectedIndex == 0)
            {
                Perh_DropSensorOn();
                Dbg_Println("Setting >> Bat cam bien roi");
            }
            else
            {
                Perh_DropSensorOff();
                Dbg_Println("Setting >> Tat cam bien roi");
            }
            /* hien thi "DONE" tren man hinh trong 1500ms */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Hien thi menu cai dat cam bien roi */
            Paint();
        }
        /* neu dang cai dat ON/OFF audio */
        else if (g_combineId == ID_AUDIO_SETTING)
        {
            if (g_selectedIndex == 0)
            {
                Perh_AudioOn();
                Dbg_Println("Setting >> Bat Audio");
            }
            else
            {
                Perh_AudioOff();
                Dbg_Println("Setting >> Tat Audio");
            }
            /* hien thi "DONE" tren man hinh trong 1500ms */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Hien thi menu cai dat audio */
            Paint();
        }
        /* neu dang cai dat ON/OFF tien tra lai */
        else if (g_combineId == ID_BILL_CHANGE)
        {
            if (g_selectedIndex == 0)
            {
                Perh_RefundOn();
                Dbg_Println("Setting >> Bat chuc nang tra lai tien");
            }
            else
            {
                Perh_RefundOff();
                Dbg_Println("Setting >> Tat chuc nang tra lai tien");
            }
            /* hien thi "DONE" tren man hinh trong 1500ms */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Hien thi menu cai dat tra lai tien */
            Paint();
        }
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat ON/OFF
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu
 */
bool OutSettingOnOff()
{
     g_visible = false;
     return g_visible;
}
