/** @file    setting_form_server_setting.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat dia chi Server
  */
 
/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "resources.h"
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
static uint8_t g_ip1 = 0, g_ip2 = 0, g_ip3 = 0, g_ip4 = 0;
static uint32_t g_port = 0;
static bool g_visible = false;
#ifdef CONTROL
static int8_t g_selectedIndex = 0;
static bool g_startEnterNumKey = false;
#endif
/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi thong tin Server len tren LCD 
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
//    char str[50];
    /* ve background */
    GLcd_ClearScreen(BLACK);
    /* ve Ip va Port vao g_offBuffer*/
    GLcd_SetFont(&font6x8);
    GLcd_DrawString("IP:", 12, 15, WHITE);
    GLcd_DrawString("PORT:", 12, 35, WHITE);

    sprintf(strLcd, "%3u.%3u.%3u.%3u", g_ip1, g_ip2, g_ip3, g_ip4);
    GLcd_DrawString(strLcd, 29, 15, WHITE);

    sprintf(strLcd, "%u", g_port);
    GLcd_DrawString(strLcd, 42, 35, WHITE);
    
    #ifdef CONTROL
    /* ve pointer khi nhap IP1 */
    if (g_selectedIndex == 0)
    {
        GLcd_FillRect(31, 24, 16, 2, WHITE);
    }
    /* ve pointer khi nhap IP2 */
    else if (g_selectedIndex == 1)
    {
        GLcd_FillRect(53, 24, 16, 2, WHITE);
    }
    /* ve pointer khi nhap IP3 */
    else if (g_selectedIndex == 2)
    {
        GLcd_FillRect(75, 24, 16, 2, WHITE);
    }
    /* ve pointer khi nhap IP4 */
    else if (g_selectedIndex == 3)
    {
        GLcd_FillRect(97, 24, 16, 2, WHITE);
    }
    /* ve pointer khi nhap PORT */
    else if (g_selectedIndex == 4)
    {
        GLcd_FillRect(30, 44, 19, 2, WHITE);
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    #endif
    GLcd_Flush();
}
/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu cai dat dia chi Server len tren LCD
 * 
 * @param    NONE
 * @retval   NONE
 */
void SettingServer_Show()
{
    /* Initialization variable */
    g_visible = true;
    #ifdef CONTROL
    g_selectedIndex = 0;
    g_startEnterNumKey = true;
    #endif
    g_ip1 = 0;
    g_ip2 = 0;
    g_ip3 = 0;
    g_ip4 = 0;
    g_port = 0;
    /* lay dia chi Server tu EEPROM */
    Perh_GetServerInfo(&g_ip1, &g_ip2, &g_ip3, &g_ip4, &g_port);
    /* Hien thi menu cai dat server */
    Paint();
}
 
/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu cai dat 
 *           tham so cua Server
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingServer_KeyPress(uint8_t keyCode)
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
        #ifdef CONTROL
        g_startEnterNumKey = true;
        g_selectedIndex++;
        if (g_selectedIndex > 4)
        {
            g_selectedIndex = 0;
            /* luu dia chi Server vao trong EEPROM */
            Perh_SaveServerInfo(g_ip1, g_ip2, g_ip3, g_ip4, g_port);
            Gsm_SetServerInfo(g_ip1, g_ip2, g_ip3, g_ip4, g_port);
            /* hien thi "DONE" tren man hinh 1500ms */
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
            Delay(1500);
        }
        /* Hien thi menu cai dat server */
        Paint();
        #endif
    }
    else    /* Numerical key */
    {
        #ifdef CONTROL
        /* bat dau nhap dia chi IP hoac truoc do nhan phim ENTER */
        if (g_startEnterNumKey == true)
        {
            g_startEnterNumKey = false;
            /* neu dang nhap IP1 */
            if (g_selectedIndex == 0)
            {
                g_ip1 = keyCode - '0';
            }
            /* neu dang nhap IP2 */
            else if (g_selectedIndex == 1)
            {
                g_ip2 = keyCode - '0';
            }
            /* neu dang nhap IP3 */
            else if (g_selectedIndex == 2)
            {
                g_ip3 = keyCode - '0';
            }
            /* neu dang nhap IP4 */
            else if (g_selectedIndex == 3)
            {
                g_ip4 = keyCode - '0';
            }
            /* neu dang nhap PORT */
            else if (g_selectedIndex == 4)
            {
                g_port = keyCode - '0';
            }
        }
        else
        {
            /* neu dang nhap IP1 */
            if (g_selectedIndex == 0)
            {
                if (g_ip1 < 100)
                {
                    g_ip1 *= 10;
                    g_ip1 += (keyCode - '0');
                }
            }
            /* neu dang nhap IP2 */
            else if (g_selectedIndex == 1)
            {
                if (g_ip2 < 100)
                {
                    g_ip2 *= 10;
                    g_ip2 += (keyCode - '0');
                }
            }
            /* neu dang nhap IP3 */
            else if (g_selectedIndex == 2)
            {
                if (g_ip3 < 100)
                {
                    g_ip3 *= 10;
                    g_ip3 += (keyCode - '0');
                }
            }
            /* neu dang nhap IP4 */
            else if (g_selectedIndex == 3)
            {
                if (g_ip4 < 100)
                {
                    g_ip4 *= 10;
                    g_ip4 += (keyCode - '0');
                }
            }
            /* neu dang nhap PORT */
            else if (g_selectedIndex == 4)
            {
                if (g_port < 36000)
                {
                    g_port *= 10;
                    g_port += (keyCode - '0');
                }
            }
        }
        /* Hien thi menu cai dat server */
        Paint();
        #endif
    }
    return true;
}

/**
 * @brief    Thoat menu cai dat dia chi Server
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu
 */
bool OutServerSetting()
{
     g_visible = false;
     return g_visible;
}
