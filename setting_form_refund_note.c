/** @file    setting_form_refund_note.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat menh gia tien tra lai
  */
 
/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "language.h"
#include "nv11.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_REFUND_NOTE
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
static uint8_t g_scroll = 0;
static bool g_visible = false;
static uint8_t g_channelNumber = 8;

static const char* g_channelValues[8] = {
    "1.000 VND",
    "2.000 VND",
    "5.000 VND",
    "10.000 VND",
    "20.000 VND",
    "50.000 VND",
    "100.000 VND",
    "200.000 VND"
};

static bool g_selectionState[8] = { false };

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief   
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    uint8_t tmp = 0;
    /* ve background */
    GLcd_ClearScreen(BLACK);
    /* ve huong dan nhan phim tren dong tren cung */
    DrawTopGuideLine();
    GLcd_SetFont(&font6x8);
    /* chi hien thi 3 menh gia tien te tren 1 page LCD */
    for (int i = g_scroll; i < g_scroll + 3; i++)
    {
        /* doi voi dong dan duoc chon thi hien thi cho mau den tren nen trang */
        if (i == g_selectedIndex)
        {
            GLcd_FillRect(19, 19 + tmp * 15, 67, 9, WHITE);
            GLcd_DrawString(g_channelValues[i], 20, 20 + tmp * 15, BLACK);
        }
        else
        {
            GLcd_DrawString(g_channelValues[i], 20, 20 + tmp * 15, WHITE);
        }
        /* neu menh gia duoc chon de hien thi tich dau "V"*/
        if (g_selectionState[i] == true)
        {
            GLcd_DrawLine(6, 23 + tmp * 15, 8, 26 + tmp * 15, WHITE);
            GLcd_DrawLine(8, 26 + tmp * 15, 12, 20 + tmp * 15, WHITE);
        }
        tmp++;
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */
 
/**
 * @brief    Hien thi menu cai dat menh gia tien tra lai
 * 
 * @param    uint8_t id channelNumber : so menh gia tien cua dau doc tien
 * @retval   NONE
 */
void SettingRefundNote_Show(uint8_t channelNumber)
{
    /* Initialization variable */
    g_channelNumber = channelNumber;
    g_visible = true;
    g_selectedIndex = 0;
    g_scroll = 0;
    /* doc menh gia tien te co trong EEPROM */
    for (int i = 0; i < channelNumber; i++)
    {
        g_selectionState[i] = Perh_GetRefundNote(i);
    }
    /* hien thi du lieu len LCD*/
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi 
 *           menu cai dat menh gia tien tra lai
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel     
 */
bool SettingRefundNote_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim mui ten xuong */
    if (keyCode == '0')
    {
        g_selectedIndex++;

        if (g_selectedIndex >= g_channelNumber)
        {
            g_selectedIndex = 0;
        }

        /* Scroll the items if needed */
        if (g_selectedIndex >= g_scroll + 3)
        {
            g_scroll = g_selectedIndex + 1 - 3;
        }
        else if (g_selectedIndex == 0)
        {
            g_scroll = 0;
        }
        /* hien thi du lieu len LCD*/
        Paint();
    }
    /* neu nhan phim mui ten len */
    else if (keyCode == '8')
    {
        g_selectedIndex--;

        if (g_selectedIndex < 0)
        {
            g_selectedIndex = g_channelNumber - 1;
        }

        /* Scroll the items if needed */
        if (g_selectedIndex < g_scroll)
        {
            g_scroll = g_selectedIndex;
        }
        else if (g_selectedIndex == g_channelNumber - 1)
        {
            g_scroll = g_channelNumber - 3;
        }
        /* hien thi du lieu len LCD*/
        Paint();
    }
    /* neu nhan phim cancel thi tra ve false va hien thi menu */
    else if (keyCode == '#')    /* CANCEL KEY */
    {
        g_visible = false;
        /* hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {
        /* dao trang thai lua chon tien tra lai 
           luu du lieu vao EEPROM */
        if (NV11_GetUnitType() == NV_TYPE_NV11)
        {
            /* clear lua chon truoc do */
        	for (int i = 0; i < g_channelNumber; i++)
		    {
				if (g_selectionState[i] == true)
                {
                    g_selectionState[i] = false;
                    Perh_SaveRefundNote(i, false);
                }
			}
            /* Them lua chon cua nguoi su dung va luu tru*/
            g_selectionState[g_selectedIndex] = true;
            Perh_SaveRefundNote(g_selectedIndex, g_selectionState[g_selectedIndex]);
            sprintf(bufferDebug, "Refund Note >> Menh gia tra lai la %s", g_channelValues[g_selectedIndex]);
            Dbg_Println(bufferDebug);
        }
        else
        {
        	g_selectionState[g_selectedIndex] = !g_selectionState[g_selectedIndex];
        	Perh_SaveRefundNote(g_selectedIndex, g_selectionState[g_selectedIndex]);
        	g_selectionState[g_selectedIndex] = Perh_GetRefundNote(g_selectedIndex);
        }
        /* hien thi du lieu len LCD*/
        Paint();
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat 
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu 
 */
bool OutSettingRefundNote()
{
     g_visible = false;
     return g_visible;
}
