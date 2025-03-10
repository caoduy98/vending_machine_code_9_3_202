/** @file    setting_form_clear_error_sell.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi xoa doanh so
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "language.h"
#include "resources.h"

/*********************************************************************************
 * EXTERN VARIABLE
 */

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();
extern void ClearSellAndCheckEmpty(void);
/*********************************************************************************
 * STATIC VARIABLE
 */

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
 * @brief    Hien thi menu xoa doanh so tren LCD
 *
 * @param    uint32_t id - not using
 * @retval   NONE
 */
void SettingClearErrorSell_Show()
{
    /* Initialization variable */
    g_visible = true;
    /* Hien thi cap nhat */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi xoa doanh so
 *
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingClearErrorSell_KeyPress(uint8_t keyCode)
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
      /* Xoa toan bo doanh so ban hang */
      //ClearSellAndCheckEmpty(); // 24_2_2025
      /* Hien thi done tren man hinh trong 1500 ms */
      GLcd_ClearScreen(BLACK);
      int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
      GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
      GLcd_Flush();
      Delay(500);
      Perh_ProcessAllWhile();
      Delay(500);
      Perh_ProcessAllWhile();
      Delay(500);
      Perh_ProcessAllWhile();
      g_visible = false;
      Setting_Show();
    }
    return true;
}

/**
 * @brief    Thoat menu hien thi xoa doanh so
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu xoa doanh so
 */
bool OutSettingClearErrorSell()
{
    g_visible = false;
    return g_visible;
}
