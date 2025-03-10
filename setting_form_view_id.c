/** @file    setting_form_view_id.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi cai dai ID may ban hang
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
#define DEBUG_LEVEL DEBUG_SETTING_FORM_VIEW_ID
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
extern void DrawPointer(int x, int y);
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
static bool g_visible = false;
static char g_machineID[6];
static uint8_t controlState = 0x00;
static uint8_t g_typeCount = 0;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi giao dien cai dat ID
 *
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
//    char str[30];

    /* ve background */
    GLcd_ClearScreen(BLACK);
    /* ve huong dan nhan phim tren dong tren cung */
    DrawTopGuideLine();

    /* Hien thi id cua may */
    GLcd_SetFont(&font6x8);
    sprintf(strLcd, "%01x-%01x-%01x-%01x-%01x-%01x",
            g_machineID[0], g_machineID[1], g_machineID[2],
            g_machineID[3], g_machineID[4], g_machineID[5]);
    int displayPosition = (GLCD_WIDTH - GLcd_MeasureString(strLcd)) / 2;
    GLcd_DrawString(strLcd, displayPosition, 35, WHITE);
    if(controlState == 0x01)
    {
      /* hien thi con tro gach duoi */
      if(g_typeCount < 6)
      {
	  GLcd_FillRect(1 + displayPosition + 12*g_typeCount, 44, 5, 2, WHITE);
      }
      else
      {
	  GLcd_FillRect(61 + displayPosition, 44, 5, 2, WHITE);
      }
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}


/*********************************************************************************
 * EXPORTED FUNCTION
 */


/**
 * @brief    Hien thi giao dien cai dat ID
 *
 * @param    NONE
 * @retval   NONE
 */
void SettingViewId_Show(uint8_t menuState)
{
    /* Initialization variable */
    g_visible = true;
    g_typeCount = 0;
    controlState = menuState;
    /* Lay ID cua may de hien thi */
    Perh_GetVendId(g_machineID);
    /* fix parameter */
    for(uint8_t index = 0; index < 6; index++)
    {
      if(g_machineID[index] > 9 || g_machineID[index] < 0) g_machineID[index] = 0;
    }
    /* Hien thi menu cai dat ID*/
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat ID
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingViewId_KeyPress(uint8_t keyCode)
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
    else if (keyCode == '*')    /* ENTER key */
    {
        if(controlState == 0x01)
        {
            /* luu tham so can cai dat vao EEPROM khi nhan enter */
            sprintf(bufferDebug, "Setting >> Cai Id may %1d:%1d:%1d:%1d:%1d:%1d", g_machineID[0],
                g_machineID[1],g_machineID[2],g_machineID[3],g_machineID[4],g_machineID[5]);
            Dbg_Println(bufferDebug);

            Perh_SetVendId(g_machineID);
            /* Viet chu "DONE" vao g_offBuffer */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            /* Hien thi LCD */
            GLcd_Flush();
            /* Hien thi chu "DONE" khoang 1500ms*/
            Delay(1500);
            /* Hien thi menu */
            Setting_Show();
            g_visible = false;
        }
    }
    else
    {
        if(controlState == 0x01)
        {
            /* reset g_machineID */
            if (g_typeCount == 0)
            {
                for (int i = 0; i < 6; i++)
                {
                    g_machineID[i] = 0;
                }
            }

            if (g_typeCount < 6)
            {
                g_machineID[g_typeCount] = keyCode - '0';
                g_typeCount++;
                /* hien thi menu cai dat ngan ban hang */
                Paint();
            }
        }
    }

    return true;
}


/**
 * @brief    Thoat menu cai dat ID
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu cai dat ID
 */
bool OutSettingViewId()
{
     g_visible = false;
     return g_visible;
}