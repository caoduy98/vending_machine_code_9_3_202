/** @file    setting_form_set_temperature.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat do am
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_SET_TEMPERATURE
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();

/*********************************************************************************
 * EXTERN VARIABLE
 */
extern uint8_t g_humidityset;

/*********************************************************************************
 * STATIC VARIABLE
 */
static bool g_visible = false;
static int g_humidity = 0;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi do am len tren LCD khong bao gom Title
 *
 * @param    NONE
 * @retval   NONE
 */
static void UpdateHumidity()
{
    /* ve mot background */
    GLcd_FillRect(0, 30, GLCD_WIDTH, 9, BLACK);
    /* ve gia tri do am vao bo dem g_offBuffer */
    GLcd_SetFont(&font6x8);
    sprintf(strLcd, "%d %", g_humidity);
    int sizeInPixel = GLcd_MeasureString(strLcd);
    GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
    /* Hien thi vung ve do am */
    GLcd_FlushRegion(30, 9);
}

/**
 * @brief    Hien thi do am len tren LCD
 *
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* ve mot background */
    GLcd_ClearScreen(BLACK);

    /* ve Title vao g_offBuffer*/
    GLcd_SetFont(&font6x8);
    const uint16_t* title = Lang_GetText(LANG_SET_HUMIDITY);
    int sizeInPixel = GLcd_MeasureStringUni(title);
    GLcd_DrawStringUni(title, (GLCD_WIDTH - sizeInPixel) / 2, 10, WHITE);
    /* hien thi gia tri do am */
    UpdateHumidity();
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu cai dat do am len tren LCD
 *
 * @param    uint32_t id : Not using
 * @retval   NONE
 */
void SettingSetHumidity_Show(uint32_t id)
{
    /* Initialization variable */
    g_visible = true;
    /* Lay gia tri do am can hien thi */
    g_humidity = Perh_GetHumidity();
    /* Hien thi menu cai dat do am */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat do am
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingSetHumidity_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }

    /* neu nhan phim cancel thi tra ve false  va hien thi menu */
    if (keyCode == '#')    /* CANCEL KEY */
    {
        g_visible = false;
        /* hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {
        /* luu tham so do am can cai dat khi nhan enter */
        sprintf(bufferDebug, "Setting >> Cai dat do am %d", g_humidity);
        Dbg_Println(bufferDebug);

        Perh_SetHumidity(g_humidity);
        g_humidityset = Perh_GetHumidity();      /* Update the humidity to the selling_main.c file */
        g_visible = false;
        /* Hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '0')    /* DOWN ARROW */
    {
        /* tang gia tri do am */
        g_humidity--;
        if (g_humidity < 5)
        {
            g_humidity = 99;
        }
        /* hien thi gia tri do am */
        UpdateHumidity();
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        /* giam gia tri do am */
        g_humidity++;
        if (g_humidity > 99)
        {
            g_humidity = 5;
        }
        /* hien thi gia tri do am */
        UpdateHumidity();
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat do am
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu cai dat do am
 */
bool OutSettingHumidity()
{
    g_visible = false;
    return g_visible;
}