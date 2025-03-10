/** @file    setting_form_set_operator_number.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi cai dat so tong dai
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "gsm.h"

/*********************************************************************************
 * DEFINE
 */
#define NO_CONTROL
/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void DrawPointer(int x, int y);
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
static bool g_visible = false;
static char g_operatorNumber[16];
#ifdef CONTROL
static uint8_t g_typeCount = 0;
#endif
/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi so tong dai tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* ve background */
    GLcd_ClearScreen(BLACK);
    DrawTopGuideLine();
#ifdef CONTROL
    /* ve gach hien thi */
    if(g_typeCount)
    {
      for(uint8_t count = g_typeCount; count < 10; count++)
      {
           GLcd_FillRect(21 + count*6, 38, 5, 2, WHITE);
      }
    }
#endif
    /* ve pointer */
    DrawPointer(5, 35);
    /* viet so tong dai vao g_offBuffer */
    GLcd_SetFont(&font6x8);
    GLcd_DrawString(g_operatorNumber, 20, 35, WHITE);
    /* hien thi con tro nhap so */
    
    /* Hien thi du lieu len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */
 
/**
 * @brief    Hien thi menu cai dat so tong dai
 * 
 * @param    NONE
 * @retval   NONE
 */
void SettingOperatorNumber_Show()
{
    /* Initialization variable */
    g_visible = true;
    #ifdef CONTROL
    g_typeCount = 0;
    #endif
    /* Lay so tong dai trong EEPROM */
    Perh_GetOperatorNumber(g_operatorNumber);
    
    for (int i = 0; i < 16; i++)
    {
        if (g_operatorNumber[i] < '0' || g_operatorNumber[i] > '9')
            g_operatorNumber[i] = 0;
    }
    /* Hien thi menu cai dat tong dai */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi 
 *           menu cai dat so tong dai
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingOperatorNumber_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim casncel thi tra ve false  va hien thi menu */
    if (keyCode == '#')    /* CANCEL KEY */
    {
        g_visible = false;
        /* hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '*')    /* ENTER key */
    {
      #ifdef CONTROL
      /* luu so tong dai vao EEPROM */
      Perh_SetOperatorNumber(g_operatorNumber);
      /* hien thi "DONE" ra ngoai man hinh trong 1500ms */
      GLcd_ClearScreen(BLACK);
      int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
      GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
      GLcd_Flush();
      Delay(1500);
      /* hien thi menu */
      Setting_Show();
      g_visible = false;
      #endif
    }
    else
    {
        #ifdef CONTROL
        if (g_typeCount == 0)
        {
            for (int i = 0; i < 16; i++)
            {
                g_operatorNumber[i] = 0;
            }
        }
        /*lay so tu ban phim */
        if (g_typeCount < 10)
        {
            g_operatorNumber[g_typeCount] = keyCode;
            g_typeCount++;
            /* Hien thi menu cai dat so tong dai */
            Paint();
        }
        #endif
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat so tong dai
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu cai dat so tong dai
 */
bool OutSettingOperatorNumber()
{
     g_visible = false;
     return g_visible;
}