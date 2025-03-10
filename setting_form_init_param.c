/** @file    setting_form_view_slave.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi khoi tao tham so
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "language.h"
#include "peripheral.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_INIT_PARAM
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * DEFINE
 */

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();
extern void FactoryReset();
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
  GLcd_ClearScreen(BLACK);
  int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_CONFIRM_RESET));
  GLcd_DrawStringUni(Lang_GetText(LANG_CONFIRM_RESET), (GLCD_WIDTH - sizeInPixel) / 2, 25, WHITE);
  
  sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_YES_NO));
  GLcd_DrawStringUni(Lang_GetText(LANG_YES_NO), (GLCD_WIDTH - sizeInPixel) / 2, 50, WHITE);
  
  GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu cap nhat bang tay tren LCD
 * 
 * @param    uint32_t id - not using
 * @retval   NONE
 */
void SettingInitParam_Show()
{
    /* Initialization variable */
    g_visible = true;
    /* Hien thi cap nhat */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cap nhat tay
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingInitParam_KeyPress(uint8_t keyCode)
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
      /* init paramter by factory reset */
      Perh_ProcessAllWhile();
      FactoryReset();
      Perh_ProcessAllWhile();
      /* init advance */
      Perh_SaveServerInfo(203, 171, 20, 62, 9201u);
      Perh_SetOperatorNumber("0931797222");
      uint8_t initPassword[6] = {'1','2','5','8','9','0'};
      Perh_SetTechPassword(initPassword);
      Perh_InitFirmwareVersionName("VM_1.3.0.bin",12);
      /* hien thi done tren man hinh trong 1500 ms */
      GLcd_ClearScreen(BLACK);
      int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
      GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
      GLcd_Flush();
      Delay(1500);
      g_visible = false;
      Dbg_Println("Setting >> Init paramter");
      Setting_Show();
    }
    return true;
}

/**
 * @brief    Thoat menu khoi tao tham so
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu khoi tao tham so
 */
bool OutSettingInitParam()
{
     g_visible = false;
     return g_visible;
}
