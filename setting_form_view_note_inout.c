/** @file    setting_form_view_note_inout.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi so tien nhan va tra
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
static bool g_visible = false;

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi so tien nhan va tra cuar may
 * 
 * @param    NONE
 * @retval   NONE
 */
void SettingViewNoteInOut_Show()
{
    /* Initialization variable */
//    char str[30];
    g_visible = true;
    uint32_t totalNoteIn = 0;
    uint32_t totalNoteOut = 0;
    
    /* lay so tien nhan va tra */
    totalNoteIn = Perh_GetTotalNoteIn();
    totalNoteOut = Perh_GetTotalNoteOut();
    /* ve background */
    GLcd_ClearScreen(BLACK);

    /* ve so tien nhan vao bo dem g_offBuffer */
    int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_TOTAL_NOTE_IN));
    GLcd_DrawStringUni(Lang_GetText(LANG_TOTAL_NOTE_IN), (GLCD_WIDTH - sizeInPixel) / 2, 0, WHITE);
    GLcd_SetFont(&font6x8);
    sprintf(strLcd, "%d", totalNoteIn);
    sizeInPixel = GLcd_MeasureString(strLcd);
    GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 20, WHITE);

    /* ve so tien trs vao bo dem g_offBuffer */
    sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_TOTAL_NOTE_OUT));
    GLcd_DrawStringUni(Lang_GetText(LANG_TOTAL_NOTE_OUT), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
    GLcd_SetFont(&font6x8);
    sprintf(strLcd, "%d", totalNoteOut);
    sizeInPixel = GLcd_MeasureString(strLcd);
    GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 48, WHITE);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi so tien
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel      
 */
bool SettingViewNoteInOut_KeyPress(uint8_t keyCode)
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
    return true;
}

/**
 * @brief    Thoat menu hien thi so tien nhan va tra
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi so tien nhan va tra
 */
bool OutSettingViewNoteInOut()
{
     g_visible = false;
     return g_visible;
}