/** @file    setting_form_set_price.c
  * @author
  * @version
  * @brief   TCung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat gia tien va so luong hang
  */


/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_SET_PRICE
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void DrawPointer(int x, int y);
extern void Setting_Show();
extern bool OutSetting();
static void TimerHandler(timer_t* tmr, void* param);

/*********************************************************************************
 * STATIC VARIABLE
 */
static int8_t g_selectedIndex = 0;
static uint32_t g_startIndex = 1;
static uint32_t g_endIndex = 1;
static uint32_t g_userValue = 0;
static bool g_visible = false;
static uint32_t g_combineId = 0;
static timer_t g_timer;
static bool g_pointerIsOn = true;
static bool g_startEnterNumKey = false;


/*********************************************************************************
 * STATIC FUNCTION
 */


/**
 * @brief    Hien thi gia tri cai dat gia tien va so luong hang
 *
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
//    char str[10];
    /* ve background */
    GLcd_ClearScreen(BLACK);

    /* ve title len g_offBuffer*/
    GLcd_SetFont(&font6x8);
    if (g_combineId == ID_SUB_PRICE)
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_PRICE),0,20,WHITE);
    }
    else if (g_combineId == ID_SUB_CAPACITY)
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_CAPACITY),0,20,WHITE);
    }
    sprintf(strLcd, "%u", g_userValue);
    GLcd_DrawString(strLcd, 73, 24, WHITE);

    /* ve pointer */
//    if (g_pointerIsOn == true)
//    {
//        if (g_selectedIndex == 0)
//        {
//            DrawPointer(25, 0);
//        }
//        else if (g_selectedIndex == 1)
//        {
//            DrawPointer(90, 0);
//        }
//        else if (g_selectedIndex == 2)
//        {
//            DrawPointer(65, 24);
//        }
//    }

    GLcd_FlushRegion(0, 45);
}


/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief Hien thi menu cai dat tien va so luong san pham trong mot ngan hang
 *
 * @param     uint32_t id   -  lua chon hien thi cai dat tien hoac so luong san pham
 * @retval    NONE
 */
void SettingSetPrice_Show(uint32_t id)
{
    /* initialization */
    g_visible = true;
    g_selectedIndex = 2;
    g_combineId = id;
    g_startIndex = 1;
    g_endIndex = 1;
    g_pointerIsOn = true;
    g_startEnterNumKey = true;
    /* lay tham so gia hoac so luong de hien thi */
    if (g_combineId == ID_SUB_PRICE)
    {
        g_userValue = Perh_GetItemPrice(g_startIndex);
    }
    else if (g_combineId == ID_SUB_CAPACITY)
    {
        g_userValue = Perh_GetItemNumber(g_startIndex);
    }

    /* ve background vao g_offBuffer */
    GLcd_ClearScreen(BLACK);

    /* ve title */
    GLcd_SetFont(&font6x8);
//    GLcd_DrawString("NO.           -", 0, 0, WHITE);
    if (g_combineId == ID_SUB_PRICE)
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_PRICE),0,20,WHITE);
    }
    else if (g_combineId == ID_SUB_CAPACITY)
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_CAPACITY),0,20,WHITE);
    }
//    /* ve gia tri ngan bat dau */
//    if (g_startIndex > 0)
//    {
//        sprintf(strLcd, "%u", g_startIndex);
//        GLcd_DrawString(strLcd, 33, 0, WHITE);
//    }
//    /* ve gia tri ngan ket thuc */
//    if (g_endIndex > 0)
//    {
//        sprintf(strLcd, "%u", g_endIndex);
//        GLcd_DrawString(strLcd, 98, 0, WHITE);
//    }
    /* ve gia tri cai dat tien hoac so luong vao g_offBuffer */
    sprintf(strLcd, "%u", g_userValue);
    GLcd_DrawString(strLcd, 73, 24, WHITE);

    /* ve con tro len man hinh */
    if (g_pointerIsOn == true)
    {
        if (g_selectedIndex == 0)       /* ve con tro khi chon ngan bat dau */
        {
            DrawPointer(25, 0);
        }
        else if (g_selectedIndex == 1)  /* ve con tro khi chon ngan ket thuc */
        {
            DrawPointer(90, 0);
        }
        else if (g_selectedIndex == 2)  /* ve con tro khi cai dat gia tri */
        {
            DrawPointer(65, 24);
        }
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();

    g_pointerIsOn = true;
}


/**
 * @brief   Ham xu ly du lieu khi co phim nhan
 *          luc LCD dang hien thi menu cai dat gia va so luong
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingSetPrice_KeyPress(uint8_t keyCode)
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
        Tmr_Stop(&g_timer);
        /* hien thi menu */
        Setting_Show();
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {
        g_startEnterNumKey = true;
        g_selectedIndex++;
        if (g_selectedIndex > 2)    /* neu nhan ENTER lan thu 3 thi luu du lieu */
        {
            g_selectedIndex = 2;

            /* luu du lieu vao mang dem */
            if (g_combineId == ID_SUB_PRICE)
            {
                sprintf(bufferDebug, "Setting >> Cai dat gia ngan %d den ngan %d gia tien %d", g_startIndex, g_endIndex, g_userValue);
                Dbg_Println(bufferDebug);
            }
            else if (g_combineId == ID_SUB_CAPACITY)
            {
                sprintf(bufferDebug, "Setting >> Cai dat so luong ngan %d den ngan %d so luong %d", g_startIndex, g_endIndex, g_userValue);
                Dbg_Println(bufferDebug);
            }

            for (int i = g_startIndex; i <= g_endIndex; i++)
            {
                if (g_combineId == ID_SUB_PRICE)
                {
                    Perh_SetItemPrice(i, g_userValue);
                }
                else if (g_combineId == ID_SUB_CAPACITY)
                {
                    Perh_SetItemNumber(i, g_userValue);
                }
            }


            /* Hien thi done tren man hinh trong 1500 ms */
              GLcd_ClearScreen(BLACK);
              int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
              GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
              GLcd_Flush();
              /* luu du lieu vao EEPROM */
              Perh_SaveItemsToEeprom();
              Delay(500);
              Perh_ProcessAllWhile();
        }

        if (g_selectedIndex == 0)        /*  nhan ENTER lan thu 3 */
        {
            g_startIndex = g_endIndex + 1;
            if(g_startIndex > 99) g_startIndex = 1;
            g_endIndex = g_startIndex;
            if (g_combineId == ID_SUB_PRICE)
            {
                g_userValue = Perh_GetItemPrice(g_startIndex);
            }
            else if (g_combineId == ID_SUB_CAPACITY)
            {
                g_userValue = Perh_GetItemNumber(g_startIndex);
            }
        }
        else if (g_selectedIndex == 1) /* neu nhan ENTER lan thu nhat */
        {
            g_endIndex = g_startIndex;
        }
        /* Hien thi menu cai dat */
        Paint();
    }
    else    /* Numerical key */
    {
        /* Neu truoc do phim duoc nhan la phim Enter */
        if (g_startEnterNumKey == true)
        {
            g_startEnterNumKey = false;
            if (g_selectedIndex == 0)      /* lay vi tri ngan bat dau */
            {
                g_startIndex = keyCode - '0';
            }
            else if (g_selectedIndex == 1) /* lay vi tri ngan ket thu */
            {
                g_endIndex = keyCode - '0';
            }
            else if (g_selectedIndex == 2) /* lay gi tri cai dat */
            {
                g_userValue = keyCode - '0';
            }
        }
        else    /* Neu truoc do phim duoc nhan khong phai phim Enter */
        {
            if (g_selectedIndex == 0)         /* lay vi tri ngan bat dau */
            {
                if(g_startIndex < 10)
                {
                    g_startIndex *= 10;
                    g_startIndex += (keyCode - '0');
                }
            }
            else if (g_selectedIndex == 1)  /* lay vi tri ngan ket thu */
            {
                if(g_endIndex < 10)
                {
                    g_endIndex *= 10;
                    g_endIndex += (keyCode - '0');
                }
            }
            else if (g_selectedIndex == 2)   /* lay gi tri cai dat */
            {
                g_userValue *= 10;
                g_userValue += (keyCode - '0');
                if (g_combineId == ID_SUB_PRICE)
                {
                    if(g_userValue > 50000) g_userValue = 50000;
                }
                else if (g_combineId == ID_SUB_CAPACITY)
                {
                    if(g_userValue > 100) g_userValue = 100;
                    if(g_userValue < 1) g_userValue = 0;
                }
            }
        }
        /* neu chua nhan ENTER 1 lan nao hoac nhan ENTER lan thu 3 */
        if (g_selectedIndex == 0)
        {
            if (g_combineId == ID_SUB_PRICE)
            {
                g_userValue = Perh_GetItemPrice(g_startIndex);
            }
            else if (g_combineId == ID_SUB_CAPACITY)
            {
                g_userValue = Perh_GetItemNumber(g_startIndex);
            }
        }
         /* Hien thi menu cai dat */
        Paint();
    }

    return true;
}

/**
 * @brief    Hient thi con tro cai dat tren LCD
 *
 * @param    timer_t* tmr  con tro toi software timer
 * @param    void* param   parameter send to function callback
 *
 * @retval   NONE
 */
static void TimerHandler(timer_t* tmr, void* param)
{
    if (g_pointerIsOn == true)
    {
        g_pointerIsOn = false;
        Tmr_Start(&g_timer, 700, TimerHandler, NULL);
    }
    else
    {
        g_pointerIsOn = true;
        Tmr_Start(&g_timer, 400, TimerHandler, NULL);
    }
    /* Hien thi menu cai dat */
    Paint();
}

/**
 * @brief    Thoat menu cai dat tien va so luong
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu ngan ban hang
 */
bool OutSettingPrice()
{
    g_visible = false;
    return g_visible;
}