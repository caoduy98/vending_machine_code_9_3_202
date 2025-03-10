/** @file    setting_form_view_error.c
  * @author  
  * @version 
  * @brief   TCung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi xem loi he thong
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
#define DEBUG_LEVEL DEBUG_SETTING_FORM_VIEW_ERROR
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
 * @brief    Hien thi loi cua may len LCD
 * 
 * @param    uint32_t id  : lua chon hien thi loi motor hay loi NV11 hoac loi cam bien roi
 * @retval   NONE
 */
void SettingViewError_Show(uint32_t id)
{
    /* Initialization variable */
//    char str[20];
    int x = 0, y = 0;
    g_visible = true;
    uint8_t count_motor_error = 0;
    /* ve background */
    GLcd_ClearScreen(BLACK);
    GLcd_SetFont(&font6x8);
    /* ve hien thi vao bo dem g_offBuffer */
    if (id == 0)    /* Neu lua chon hien thi loi motor */
    {
        for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
        {
            /* if motor error then draw on LCD */
            if (Perh_ItemIsExist(i) && Perh_ItemIsLocked(i))
            {
                count_motor_error++;
                sprintf(strLcd, "%02d", i);
                GLcd_DrawString(strLcd, x, y, WHITE);
                x += 15;
                if (x > 128 - 13)
                {
                    x = 0;
                    y += 11;
                }
            }
        }
        if(count_motor_error == 0)
        {
          GLcd_DrawStringUni(Lang_GetText(LANG_NO_ERROR), 0, 0, WHITE);
          Dbg_Println("Setting >> Khong co loi motor");
        }
    }
    else if (id == 1)   /* Neu lua chon hien thi loi NV11 */
    {
        /* Neu NV11 bi loi thi hien thi thong bao loi */
        if (NV11_GetLatestEvent() == NV11_ERROR)
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_NV11_ERROR), 0, 0, WHITE);
            Dbg_Println("Setting >> Loi NV11");
        }
        else /* Neu NV11 khong bi loi thi hien thi khong loi */
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_NO_ERROR), 0, 0, WHITE);
            Dbg_Println("Setting >> NV11 khong loi");
        }
    }
    else if (id == 2)   /* Neu lua chon hien thi loi cam bien roi */
    {
        if (Perh_GetErrorDropSensor() == 0x01)
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_DROP_SENSOR_ERROR), 0, 0, WHITE);
            Dbg_Println("Setting >> Loi cam bien roi");
        }
        else
        {
            GLcd_DrawStringUni(Lang_GetText(LANG_NO_ERROR), 0, 0, WHITE);
            Dbg_Println("Setting >> Cam bien roi khong loi");
        }
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi loi
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel   
 */
bool SettingViewError_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }

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

    }
    else if (keyCode == '8')    /* UP ARROW */
    {

    }
    return true;
}

/**
 * @brief    Thoat menu xem loi
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu ngan ban hang
 */
bool OutSettingViewError()
{
     g_visible = false;
     return g_visible;
}