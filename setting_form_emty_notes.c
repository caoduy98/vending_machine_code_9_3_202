/** @file    setting_form_emty_notes.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu lay tien tra lai ra
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
#define DEBUG_LEVEL DEBUG_SETTING_FORM_EMTY_NOTE
#if DEBUG_LEVEL
#define Dbg_Println(x)
#endif
/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */
static bool g_visible = false;
//extern uint8_t flag_ctrl_motor_provider; //Them 4/3/2025

/*********************************************************************************
 * EXPORTED FUNCTION
 */
 
/**
 * @brief    Hien thi menu lay tien tra lai ra
 * 
 * @param    NONE
 * @retval   NONE
 */
void SettingEmptyNote_Show()
{
    /* Initialization variable */
    uint32_t tick = 0;
    //char str[20];
    g_visible = true;
    /* ve mot background */
    GLcd_ClearScreen(BLACK);
    GLcd_DrawStringUni(Lang_GetText(LANG_EMPTY_RECYCLER),18,10,WHITE);
    /* Tong so tien co trong NoteFloat */
    int availableChange = NV11_GetAvailableChange();
    sprintf(strLcd, "%d", availableChange);
    GLcd_DrawString(strLcd, 45, 30, WHITE);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
    /* cho phep dau doc tien hoat dong */
    NV11_Enable();
    /* cho phep thiet bi tra lai tien */
    NV11_EnablePayout();
    /* chuyen tien trong floatnote vao cashbox */
    NV11_EmptyNotes();
    tick = GetTickCount();
    while (1)
    {
        if (GetTickCount() - tick > 500)
        {
            /* hien thi so tien con trong floatnote */
            tick = GetTickCount();
            availableChange = NV11_GetAvailableChange();
            sprintf(strLcd, "%d", availableChange);
            GLcd_FillRect(45, 30, 50, 10, BLACK);
            GLcd_DrawString(strLcd, 45, 30, WHITE);
            GLcd_Flush();
            /* neu so tien con lai bang khong thi thoat khoi vong lap */
            if (availableChange == 0)
            {
                break;
            }
        }
        NV11_Process();

        /* Check tick all time */
        Perh_ProcessAllWhile();
    }
    /* hien thi lai menu */
    Delay(5000);
    Dbg_Println("Setting >> Lay het tien tra lai trong floatnote");
    g_visible = false;
    Setting_Show();
    /* vo hieu hoa dau doc tien */
    NV11_Disable();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi
 *           menu lay tien tra lai ra
 * 
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel    
 */
bool SettingEmptyNote_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }

    if (keyCode == '#')    /* CANCEL KEY */
    {

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
 * @brief    Thoat menu lay tien tra lai
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu
 */
bool OutSettingEmptyNote()
{
     g_visible = false;
     return g_visible;
}