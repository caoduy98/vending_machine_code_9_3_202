/** @file    setting_form_set_slot_number.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat ngan ban hang
  */

/*********************************************************************************
 * INCLUDE
 */

#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "resources.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_SET_SLOT_NUMBER
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
static uint32_t g_columnNumber = 1;
static uint32_t g_rowNumber = 1;
static bool g_visible = false;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi giao dien menu ngan ban hang tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* Tao background */
    GLcd_ClearScreen(BLACK);

    /* hien thi chu theo bang language */
    int sizeInPixelColumn = GLcd_MeasureStringUni(Lang_GetText(LANG_COLUMN_NUMBER));
    int sizeInPixelRow = GLcd_MeasureStringUni(Lang_GetText(LANG_ROW_NUMBER));
    int maxSizeInPixel = 0;
    if (sizeInPixelColumn > sizeInPixelRow)
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_COLUMN_NUMBER), 15, 15, WHITE);
        GLcd_DrawStringUni(Lang_GetText(LANG_ROW_NUMBER), 15 + sizeInPixelColumn - sizeInPixelRow, 30, WHITE);
        maxSizeInPixel = sizeInPixelColumn;
    }
    else
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_COLUMN_NUMBER), 15 + sizeInPixelRow - sizeInPixelColumn, 15, WHITE);
        GLcd_DrawStringUni(Lang_GetText(LANG_ROW_NUMBER), 15, 30, WHITE);
        maxSizeInPixel = sizeInPixelRow;
    }
    
    /* dua ve font 6x8 de hien thi so dong va so cot */
    GLcd_SetFont(&font6x8);
    
    /* viet so cot vao bo dem g_offBuffer */
    sprintf(strLcd, "%u", g_columnNumber);
    GLcd_DrawString(strLcd, 18 + maxSizeInPixel, 21, WHITE);
    
    /* viet so cot vao bo dem g_offBuffer */
    sprintf(strLcd, "%u", g_rowNumber);
    GLcd_DrawString(strLcd, 18 + maxSizeInPixel, 36, WHITE);

    if (g_selectedIndex == 0)         /* hien thi con tro khi dang hien thi column */
    {
        DrawPointer(5, 21);
    }
    else if (g_selectedIndex == 1)   /* hien thi con tro khi dang hien thi row */
    {
        DrawPointer(5, 36);
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu cai dat so ngan ban hang
 * 
 * @param    NONE
 * @retval   NONE
 */
void SettingSlotNumber_Show()
{
    /* Initialization variable */
    g_visible = true;
    g_selectedIndex = 0;
    /* Lay tham so hang va cot */
    g_columnNumber = Perh_GetSlotColumn();
    g_rowNumber = Perh_GetSlotRow();
    /* Display menu setting number slot item */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu ban hang
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingSlotNumber_KeyPress(uint8_t keyCode)
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
        g_selectedIndex++;
        if (g_selectedIndex > 1)
        {
            g_selectedIndex = 0;
            /* luu tham so hang va cot khi nhan enter */
            sprintf(bufferDebug, "Setting >> Cai dat so hang %d - so cot %d", g_rowNumber, g_columnNumber);
            Dbg_Println(bufferDebug);

            Perh_SetSlotColumn(g_columnNumber);
            Perh_SetSlotRow(g_rowNumber);
            Perh_UpdateItemId();
            /* Viet chu "DONE" vao g_offBuffer */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            /* Hien thi LCD */
            GLcd_Flush();
            /* Hien thi chu "DONE" khoang 1500ms*/
            Delay(1500);
        }
        /* hien thi menu cai dat ngan ban hang */
        Paint();
    }
    else if (keyCode == '8' || keyCode == '0') /* Numerical key */
    {
        if (g_selectedIndex == 0)
        {
        if(g_columnNumber == 6) g_columnNumber = 10;
        else if(g_columnNumber == 10) g_columnNumber = 6;
        }
        else if (g_selectedIndex == 1)
        {
        g_rowNumber = 6;
        }
        Paint();
    }
    /* hien thi giao dien menu */
    return true;
}

/**
 * @brief    Thoat menu cai dat ngan ban hang
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu ngan ban hang
 */
bool OutSettingSlot()
{
    g_visible = false;
    return g_visible;
}
