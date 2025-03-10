/** @file    setting_form_test_slot.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu test slot MBH
  */

/*********************************************************************************
 * INCLUDE
 */

#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "peripheral.h"
#include "motor.h"
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_FORM_TEST_SLOT
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * MACRO
 */
//Set bit
#define SET_BIT(a, n) ((a)[(n) / 8] |= (1 << ((n) % 8)))
//Reset bit
#define RESET_BIT(a, n) ((a)[(n) / 8] &= ~(1 << ((n) % 8)))
//Read bit
#define READ_BIT(a, n) ((a[(n) / 8] >> ((n) % 8)) & 1)

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void DrawPointer(int x, int y);
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
static uint8_t g_selectedIndexStart = 0;
static uint8_t g_selectedTest = 0;
static uint8_t g_selectedIndexStop = 0;
static bool g_visible = false;
static bool g_testStatus = false;
static uint8_t g_enterStatus = 0;
static uint8_t g_errorSlot[8] = {0};
static uint8_t g_errorNum = 0;
static bool g_startEnterNumKey = false;
static bool g_testDone = false;
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
//    char str[10];
    int x = 56, y = 32;
    /* ve background */
    GLcd_ClearScreen(BLACK);

    /* ve title len g_offBuffer*/
    GLcd_SetFont(&font6x8);
    GLcd_DrawString("NO.           -", 0, 0, WHITE);

    if (g_selectedIndexStart > 0)
    {
        sprintf(strLcd, "%u", g_selectedIndexStart);
        GLcd_DrawString(strLcd, 33, 0, WHITE);
    }

    if (g_selectedIndexStop > 0)
    {
        sprintf(strLcd, "%u", g_selectedIndexStop);
        GLcd_DrawString(strLcd, 98, 0, WHITE);
    }

    /* ve pointer */
    if (g_enterStatus == 0)
    {
        DrawPointer(25, 0);
    }
    else if (g_enterStatus == 1)
    {
        DrawPointer(90, 0);
    }

    if (g_testStatus == true)
    {
        /* Hien thi test slot */
        if(g_testDone == true)
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE),0,10,WHITE);
        }
        else
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_TESTING_SLOT),0,10,WHITE);
            sprintf(strLcd, "%u", g_selectedTest);
            GLcd_DrawString(strLcd, 110, 16, WHITE);
        }

        /* Hien thi ngan loi */
        if(g_errorNum > 0)
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_ERROR_SLOT),0,26,WHITE);
            for (uint8_t index = 0; index < 64; index++)
            {
                if(READ_BIT(g_errorSlot, index))
                {
                    sprintf(strLcd, "%02d", index);
                    GLcd_DrawString(strLcd, x, y, WHITE);
                    x += 15;
                    if (x > 128 - 13)
                    {
                        x = 0;
                        y += 11;
                    }
                }
            }
        }
        else
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_NO_ERROR),0,26,WHITE);
        }
    }
    GLcd_Flush();
}

static void ClearError()
{
    g_errorNum = 0;
    for (uint8_t index = 0; index < 8; index++)
    {
        g_errorSlot[index] = 0x00;
    }
}
/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu test slot
 *
 * @param    NONE
 * @retval   NONE
 */
void SettingTestSlot_Show()
{
    /* Initialization variable */
    g_visible = true;
    g_selectedIndexStart = 1;
    g_selectedIndexStop = 1;
    g_selectedTest = 0;
    g_enterStatus = 0;
    g_startEnterNumKey = true;
    g_testStatus = false;
    g_testDone = false;
    ClearError();
    /* Display menu setting number slot item */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu test slot
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingTestSlot_KeyPress(uint8_t keyCode)
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
        if (g_testStatus == false)
        {
            /* hien thi menu */
            Setting_Show();
        }
        else if (g_testStatus == true)
        {
            Dbg_Println("Setting >> Ket thuc test slot");
        }
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {
        g_startEnterNumKey = true;
        if (g_enterStatus == 0)// && g_selected != 0) /* Nhan Enter de luu vi tri bat dau */
        {
            g_enterStatus = 1;
            /* hien thi menu cai dat ngan ban hang */
            Paint();
        }
        else if (g_enterStatus == 1)// && g_selected != 0) /* Nhan Enter de luu vi tri stop sau do tien hanh test */
        {
            g_selectedTest = g_selectedIndexStart;
            /* Bat dau Test */
            ClearError();
            g_testStatus = true;
            while(g_selectedTest <= g_selectedIndexStop)
            {
                /* Hien thi vi tri test tren man hinh */
                Paint();
                /* chay motor */
                bool res = false;//MotorRun();
                if( res == false )
                {
                    SET_BIT(g_errorSlot, g_selectedTest);
                    g_errorNum++;
                }
                g_selectedTest++;
                Delay(1000);

                /* Check tick all time */
                Perh_ProcessAllWhile();
            }
            g_testDone = true;
            g_enterStatus = 0;
            /*hien thi ket qua ra man hinh LCD sau do reset bien */
            Paint();
            g_testStatus = false;
            g_testDone = false;
        }
    }
    else
    {
        if(g_startEnterNumKey == true)
        {
            if(g_enterStatus == 0)
            {
                g_selectedIndexStart = keyCode - '0';
            }
            else
            {
                g_selectedIndexStop = keyCode - '0';
            }
            g_startEnterNumKey = false;
        }
        else
        {
            if (g_enterStatus == 0)
            {
                if(g_selectedIndexStart < 10)
                {
                    g_selectedIndexStart *= 10;
                    g_selectedIndexStart += (keyCode - '0');
                }
                /* gioi han vi tri tesst */
                if (g_selectedIndexStart > 60)
                {
                    g_selectedIndexStart = 60;
                }
            }
            else if (g_enterStatus == 1)
            {
                if(g_selectedIndexStop < 10)
                {
                    g_selectedIndexStop *= 10;
                    g_selectedIndexStop += (keyCode - '0');
                }
                /* gioi han vi tri tesst */
                if (g_selectedIndexStop > 60)
                {
                    g_selectedIndexStop = 60;
                }
            }
        }
        Paint();
    }
    /* hien thi giao dien menu */
    return true;
}

/**
 * @brief    Thoat menu test slot MBH
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu
 */
bool OutSettingTestSlot()
{
     g_visible = false;
     return g_visible;
}
