/** @file    setting_form_view_version.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu firmware
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "language.h"
#include "peripheral.h"

/*********************************************************************************
 * DEFINE
 */

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */

//static int g_combindId;
static bool g_visible = false;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi cap nhat len tren LCD 
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* init variable */
    uint8_t firmwareVersion[3] = {0};
//    uint8_t buf_str[20];
    /* Draw background */
    GLcd_ClearScreen(BLACK);
        /* ve Title vao g_offBuffer*/
    GLcd_SetFont(&font6x8);
    const uint16_t* title = Lang_GetText(LANG_FIRMWARE_CURRENT);
    int sizeInPixel = GLcd_MeasureStringUni(title);
    GLcd_DrawStringUni(title, (GLCD_WIDTH - sizeInPixel) / 2, 10, WHITE);
    /* get name firmware version */
    Perh_GetFirmwareVersion(firmwareVersion);
    
    /* ve version tren background */
    sprintf(strLcd, "VM_%d.%d.%d", firmwareVersion[0], firmwareVersion[1], firmwareVersion[2]);
    sizeInPixel = GLcd_MeasureString(strLcd);
    GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi ten version tren LCD
 * 
 * @param    uint32_t id - not using
 * @retval   NONE
 */
void SettingViewFirmwareVer_Show()
{
    /* Initialization variable */
    g_visible = true;
    /* Hien thi cap nhat */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi version menu
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingViewFirmwareVer_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim cancel thi tra ve false  va hien thi menu */
    if (keyCode == '#')             /* CANCEL KEY */
    {
      g_visible = false;
      /* hien thi menu */
      Setting_Show();
      return true;
    }
    else if (keyCode == '*')       /* ENTER KEY */
    {
     
    }
    return true;
}

/**
 * @brief    Thoat menu hien thi version
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi version menu
 */
bool OutSettingViewFirmwareVer()
{
     g_visible = false;
     return g_visible;
}
