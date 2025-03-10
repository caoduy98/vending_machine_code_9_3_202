/** @file    setting_form_view_bill_change.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi so tien tra lai trong floatnote
  */

/*********************************************************************************
 * INCLUDE
 */

#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "nv11.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_VIEW_BILL_CHANGE
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
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
 * @brief    Hien thi tong so tien tra lai trong floatnot
 * 
 * @param    NONE
 * @retval   NONE
 */
void SettingViewBillChange_Show()
{
    /* Initialization variable */
//    char str[20];
    g_visible = true;
    /* ve mot background */
    GLcd_ClearScreen(BLACK);
    /* ve title */
    int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_BILL_CHANGE));
    GLcd_DrawStringUni(Lang_GetText(LANG_BILL_CHANGE), (GLCD_WIDTH - sizeInPixel) / 2, 15, WHITE);
    /* Lay tong so tien tra lai dduoc luu tru trong dau doc tien */
    int availableChange = NV11_GetAvailableChange();
    /* ve tong so tien tra lai trong g_offbuffer */
    sprintf(strLcd, "%d", availableChange);
    GLcd_SetFont(&font6x8);
    sizeInPixel = GLcd_MeasureString(strLcd);
    GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 35, WHITE);
    /* doi voi dau doc tien NV200 thì hien thi nhan phim "0" */
    if (NV11_GetUnitType() == NV_TYPE_NV200)
    {
        GLcd_DrawString("(0)==>", 85, 55, WHITE); 
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();

    sprintf(bufferDebug, "Setting >> So tien tra lai %d", availableChange);
    Dbg_Println(bufferDebug);
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi tong so tien tra lai
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel   
 */
bool SettingViewBillChange_KeyPress(uint8_t keyCode)
{
    /* Initialization variable */
//    char str[20];
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

    }
    else if (keyCode == '0')    /* DOWN ARROW */
    {
        /* chi ap dung voi dau doc tien NV200 */
        /* nhan phim "0" de xem chi tiet tien tra lai */
        if (NV11_GetUnitType() == NV_TYPE_NV200)
        {
            /* ve background */
            GLcd_ClearScreen(BLACK);
            GLcd_SetFont(&font6x8);
            /* lay so menh gia co trong to tien */
            int channelNumber = NV11_GetChannelNumber();
            /* hien thi so to tien theo menh gia */
            for (int channelIndex = 0; channelIndex < channelNumber; channelIndex++)
            {
                sprintf(strLcd, "%dK:%d", NV11_GetChannelValue(channelIndex) / 1000, NV11_GetStoredNoteByChannel(channelIndex));
                if (channelIndex < 3)
                {
                    GLcd_DrawString(strLcd, 6, channelIndex * 13 + 3, WHITE);
                }
                else if (channelIndex < 5)
                {
                    GLcd_DrawString(strLcd, 0, channelIndex * 13 + 3, WHITE);
                }
                else if (channelIndex < 6)
                {
                    GLcd_DrawString(strLcd, 85, (channelIndex - 5) * 13 + 3, WHITE);
                }
                else
                {
                    GLcd_DrawString(strLcd, 79, (channelIndex - 5) * 13 + 3, WHITE);
                }
            }
            GLcd_DrawString("<==(8)", 85, 55, WHITE);
            /* dua du lieu tu bo dem g_offBuffer len LCD */
            GLcd_Flush();
        }
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        /* chi ap dung voi dau doc tien NV200 */
        /* nhan phim so "8" tro ve menu tong so tien tra lai */
        if (NV11_GetUnitType() == NV_TYPE_NV200)
        {
            /* ve background */
            GLcd_ClearScreen(BLACK);
            /* ve title */
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_BILL_CHANGE));
            GLcd_DrawStringUni(Lang_GetText(LANG_BILL_CHANGE), (GLCD_WIDTH - sizeInPixel) / 2, 15, WHITE);
            /* Lay tong so tien tra lai dduoc luu tru trong dau doc tien */
            int availableChange = NV11_GetAvailableChange();
            /* ve tong so tien tra lai trong g_offbuffer */
            sprintf(strLcd, "%d", availableChange);
            GLcd_SetFont(&font6x8);
            sizeInPixel = GLcd_MeasureString(strLcd);
            GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 35, WHITE);
            GLcd_DrawString("(0)==>", 85, 55, WHITE);
            /* dua du lieu tu bo dem g_offBuffer len LCD */
            GLcd_Flush();
        }
    }

    return true;
}

/**
 * @brief    Thoat menu xem tong so tien tra lai
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu
 */
bool OutSettingViewBillChange()
{
     g_visible = false;
     return g_visible;
}