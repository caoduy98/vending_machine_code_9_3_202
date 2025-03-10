/** @file    setting_form_volume_setting.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat am luong cuoc goi & media
  *          & muc thu am cua mic
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "language.h"
#include "gsm.h"
#include "wav_player.h"
 /* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_VOLUME_SETTING
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
static uint8_t g_level;
static uint8_t g_maxLevel;
static uint8_t g_combineId = 0;

/*********************************************************************************
 * CONSTANT
 */
static const uint8_t width = 114;
static const uint8_t height = 2;
static const uint8_t left = 7;
static const uint8_t top = 49;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi slide va gia tri am luong tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
    /* Draw background */
    GLcd_FillRect(0, top - 12, GLCD_WIDTH, GLCD_HEIGHT - top + 12, BLACK);
    /* Draw the slide bar */
    GLcd_FillRect(left, top, width, height, WHITE);
    /* Ve gia tri min-max cua slide bar */
    GLcd_SetFont(&font6x8);
//    char str[10];
    GLcd_DrawString("0", left, top - 12, WHITE);
    sprintf(strLcd, "%d", g_maxLevel);
    GLcd_DrawString(strLcd, left + width - GLcd_MeasureString(strLcd), top - 12, WHITE);
    /* ve con tro tren slide bar*/
    int x = g_level * (width - height) / g_maxLevel + left;
    GLcd_FillRect(x, top - 2, height, height * 3, WHITE);
    sprintf(strLcd, "%d", g_level);
    GLcd_DrawString(strLcd, x - GLcd_MeasureString(strLcd) / 2, top + height + 6, WHITE);
    GLcd_FlushRegion(top - 12, GLCD_HEIGHT - top + 12);
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */
 
/**
 * @brief    Hien thi menu cai dat am luong len tren LCD
 * 
 * @param    const uint16_t* title   -   Con tro den text title de hien thi LCD
 * @param    uint32_t id             -   Lua chon keu menu hien thi cai dat la am luong cuoc goi
 *                                       am luong media hay am luong mic
 * @retval   NONE
 */
void SettingVolume_Show(const uint16_t* title, uint32_t id)
{
    /* Initialization variable */
    g_visible = true;
    g_maxLevel = 10;
    if (id == 0)            /* Main volume */
    {
        g_combineId = 0;
        g_level = Perh_GetMediaVolume();
    }
    else if (id == 1)      /* Call volume */
    {
        g_combineId = 1;
        g_level = Perh_GetCallVolume();
    }
    else                  /* Microphone gain level */
    {
        g_combineId = 2;
        g_level = Perh_GetMicrophoneLevel();
    }
    /* ve background */
    GLcd_ClearScreen(BLACK);
    /* ve huong dan nhan phim tren dong tren cung */
    DrawTopGuideLine();
    /* Ve title */
    int x = GLcd_CalculateXCenter(title);
    GLcd_DrawStringUni(title, x, 15, WHITE);
    /* ve mot slide bar */
    GLcd_FillRect(left, top, width, height, WHITE);
    GLcd_SetFont(&font6x8);
//    char str[10];
    GLcd_DrawString("0", left, top - 12, WHITE);
    sprintf(strLcd, "%d", g_maxLevel);
    GLcd_DrawString(strLcd, left + width - GLcd_MeasureString(strLcd), top - 12, WHITE);
    /* ve con tro tren slide bar*/
    x = g_level * (width - height) / g_maxLevel + left;
    GLcd_FillRect(x, top - 2, height, height * 3, WHITE);
    sprintf(strLcd, "%d", g_level);
    GLcd_DrawString(strLcd, x - GLcd_MeasureString(strLcd) / 2, top + height + 6, WHITE);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat am luong
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingVolume_KeyPress(uint8_t keyCode)
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
        if (g_combineId == 0)   /* Main volume */
        {
            /* thiet lap cai dat am luong cho media 
               luu tham so cai dat vao EEPROM   */
            sprintf(bufferDebug, "Setting >> Am luong media %d", g_level);
            Dbg_Println(bufferDebug);
            Wav_SetVolume(g_level);
            Perh_SetMediaVolume(g_level);
        }
        else if (g_combineId == 1)  /* Call volume */
        {
            /* thiet lap cai dat am luong cuoc goi 
               luu tham so cai dat vao EEPROM   */
            sprintf(bufferDebug, "Setting >> Am luong cuoc goi %d", g_level);
            Dbg_Println(bufferDebug);
            Gsm_SetAudioLevel(g_level);
            Perh_SetCallVolume(g_level);
        }
        else
        {
            /* thiet lap cai dat am luong mic
               luu tham so cai dat vao EEPROM   */
            sprintf(bufferDebug, "Setting >> Do nhay mic %d", g_level);
            Dbg_Println(bufferDebug);
            Gsm_SetMicrophoneLevel(g_level);
            Perh_SetMicrophoneLevel(g_level);
        }
    }
    else if (keyCode == '0')    /* DOWN */
    {
        if (g_level > 0)
        {
            g_level--;
        }

        if (g_combineId == 0)   /* Main volume */
        {
            /* thiet lap cai dat am luong cho media 
               luu tham so cai dat vao EEPROM   */
            Wav_SetVolume(g_level);
            Perh_SetMediaVolume(g_level);
        }
        else if (g_combineId == 1)  /* Call volume */
        {
            /* thiet lap cai dat am luong cuoc goi 
               luu tham so cai dat vao EEPROM   */
            Gsm_SetAudioLevel(g_level);
            Perh_SetCallVolume(g_level);
        }
        else
        {
            /* thiet lap cai dat am luong mic
               luu tham so cai dat vao EEPROM   */
            Gsm_SetMicrophoneLevel(g_level);
            Perh_SetMicrophoneLevel(g_level);
        }

        Paint();
    }
    else if (keyCode == '8')    /* UP */
    {
        if (g_level < g_maxLevel)
        {
            g_level++;
        }

        if (g_combineId == 0)   /* Main volume */
        {
            /* thiet lap cai dat am luong cho media 
               luu tham so cai dat vao EEPROM   */
            Wav_SetVolume(g_level);
            Perh_SetMediaVolume(g_level);
        }
        else if (g_combineId == 1)  /* Call volume */
        {
            /* thiet lap cai dat am luong cuoc goi 
               luu tham so cai dat vao EEPROM   */
            Gsm_SetAudioLevel(g_level);
            Perh_SetCallVolume(g_level);
        }
        else
        {
            /* thiet lap cai dat am luong mic
               luu tham so cai dat vao EEPROM   */
            Gsm_SetMicrophoneLevel(g_level);
            Perh_SetMicrophoneLevel(g_level);
        }
        /* Hien thi menu cai dat volume */
        Paint();
    }
    return true;
}

/**
 * @brief    Ham xu ly du lieu khi co giu phim luc LCD dang hien thi menu cai dat am luonh
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi
 */
bool SettingVolume_KeyHold(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }

    if (keyCode == '0')    /* DOWN */
    {
        if (g_level > 0)
        {
            g_level--;
        }
        /* Hien thi menu cai dat volume */
        Paint();
    }
    else if (keyCode == '8')    /* UP */
    {
        if (g_level < g_maxLevel)
        {
            g_level++;
        }
        /* Hien thi menu cai dat volume */
        Paint();
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat am luong
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu am luong
 */
bool OutSettingVolume()
{
     g_visible = false;
     return g_visible;
}