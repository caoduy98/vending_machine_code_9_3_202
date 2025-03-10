/** @file    setting_form_accept_note.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat menh gia tien lon nhat may chap nhan
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
#define DEBUG_LEVEL DEBUG_SETTING_FORM_ACCESPT_NOTE
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
static uint16_t g_selectedNote = 0;
static int8_t g_selectedIndex = 0;
static bool g_visible = false;
static uint8_t g_scroll = 0;
static const uint8_t g_channelNumber = 9;

static const char* g_channelValues[9] = {
    "1.000 VND",
    "2.000 VND",
    "5.000 VND",
    "10.000 VND",
    "20.000 VND",
    "50.000 VND",
    "100.000 VND",
    "200.000 VND",
    "500.000 VND"
};

static bool g_selectionState[9] = { false };
/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi menh gia tien lon nhat ma dau doc tien chap nhan len tren LCD
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
 * @brief    Hien thi menu cai dat menh gia tien ma dau doc tien se nhan
 *
 * @param    NONE
 * @retval   NONE
 */
void SettingAcceptNote_Show()
{
    /* Initialization variable */
    g_visible = true;
    g_selectedIndex = 0;
    /* lay tham so gia tien lon nhat may chap nhan trong EEPROM */
    /* bit 8 | bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 */
    /* 500K  | 200k  |  100k |   50k |  20k  |   10k |   5k  |   2k  |   1k  */
    g_selectedNote = Perh_GetAcceptableNoteHight();
    g_selectedNote <<= 8;
    g_selectedNote |= Perh_GetAcceptableNoteLow();

    for (int i = 0; i < g_channelNumber; ++i)
    {
        if ((g_selectedNote >> i) & 0x01)
        {
            g_selectionState[i] = true;
        }
    }
    /* Hien thi menh gia tien lon nhat may chap nhan */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat menh gia tien chap nhan
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingAcceptNote_KeyPress(uint8_t keyCode)
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

        /* Hien thi menu */
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
        /* Hien thi menu */
        Paint();
    }
    /* neu nhan phim cancel thi tra ve false  va hien thi menu */
    else if (keyCode == '#')    /* CANCEL KEY */
    {
        g_visible = false;
        Setting_Show();
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {
        /* Luu tru cac menh gia tien nhan vao eeprom */
        g_selectedNote = 0x00;
        g_selectionState[g_selectedIndex] = !g_selectionState[g_selectedIndex];
        for (int i = 0; i < g_channelNumber; ++i)
        {
            if (g_selectionState[i])
            {
                g_selectedNote |= (0x01 << i);
            }
        }
        Perh_SaveAcceptableNoteLow(g_selectedNote & 0xFF);
        Perh_SaveAcceptableNoteHigh((g_selectedNote >> 8) & 0xFF);
        GLcd_ClearScreen(BLACK);

        if(NV11_SetAcceptNote(g_selectedNote) == 0)
        {
            /* Display lcd */
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            /* Debug */
            for (int i = 0; i < g_channelNumber; ++i)
            {
                if(g_selectionState[i])
                {
                    sprintf(bufferDebug, "Setting >> Menh gia tien chap nhan %s", g_channelValues[i]);
                    Dbg_Println(bufferDebug);
                }
            }
        }
        else
        {
            g_selectionState[g_selectedIndex] = !g_selectionState[g_selectedIndex];
            /* Display lcd */
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_ACCEPT_ERROR));
            GLcd_DrawStringUni(Lang_GetText(LANG_ACCEPT_ERROR), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            /* Debug */
            Dbg_Println("Setting >> Cai dat menh gia chap nhan that bai");
        }
        /* Hien thi "DONE" tren man hinh trong 1500ms */

        GLcd_Flush();
        Delay(1500);
        /* Hien thi menu */
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
bool OutSettingAcceptNote()
{
    g_visible = false;
    return g_visible;
}
