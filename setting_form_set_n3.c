/** @file    setting_form_set_n3.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat so luong huong ban them
  *          khi co tin hieu het huong cua cam bien
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
extern uint32_t g_n3_set;
/*********************************************************************************
 * STATIC VARIABLE
 */
static bool g_visible = false;
static int g_n3 = 0;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi n3 len tren LCD khong bao gom title
 *
 * @param    NONE
 * @retval   NONE
 */
static void UpdateN3Screen()
{
    /* ve mot background */
    GLcd_FillRect(0, 30, GLCD_WIDTH, 9, BLACK);
    /* ve gia tri cua n3 */
    GLcd_SetFont(&font6x8);
    sprintf(strLcd, "%d", g_n3);
    int sizeInPixel = GLcd_MeasureString(strLcd);
    GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
    GLcd_FlushRegion(30, 9);
}

/**
 * @brief    Hien thi n3 len tren LCD
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
    const uint16_t* title = Lang_GetText(LANG_N3_CAPACITY);
    int sizeInPixel = GLcd_MeasureStringUni(title);
    GLcd_DrawStringUni(title, (GLCD_WIDTH - sizeInPixel) / 2, 10, WHITE);
    /* hien thi gia tri n3 */
    UpdateN3Screen();
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu cai dat N3 len tren LCD
 *
 * @param    uint32_t id : Not using
 * @retval   NONE
 */
void SettingSetN3_Show(uint32_t id)
{
    /* Initialization variable */
    g_visible = true;
    /* Lay gia tri N3 can hien thi */
    g_n3 = Perh_GetN3Capacity();
    /* Hien thi menu cai dat n3 */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat N1
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingSetN3_KeyPress(uint8_t keyCode)
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
        /* luu tham so n3 can cai dat khi nhan enter */
        sprintf(bufferDebug, "Setting >> Cai dat n3 %d", g_n3);
        Dbg_Println(bufferDebug);

        Perh_SetN3Capacity(g_n3);
        g_n3_set = Perh_GetN3Capacity();      /* Update the n1 to the selling_main.c file */
        g_visible = false;
        /* Hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '0')    /* DOWN ARROW */
    {
        /* tang gia tri n3 */
        g_n3--;
        if (g_n3 < 1)
        {
            g_n3 = 500;
        }
        /* hien thi gia tri n1 */
        UpdateN3Screen();
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        /* giam gia tri n3 */
        g_n3++;
        if (g_n3 > 500)
        {
            g_n3 = 1;
        }
        /* hien thi gia tri n1 */
        UpdateN3Screen();
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat n3
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu cai dat n1
 */
bool OutSettingN3Capacity()
{
    g_visible = false;
    return g_visible;
}