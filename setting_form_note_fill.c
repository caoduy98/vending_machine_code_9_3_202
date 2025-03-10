/** @file    setting_form_note_fill.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu dua tien tra lai vao may
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
#define DEBUG_LEVEL DEBUG_SETTING_FORM_INIT_PARAM
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
static bool g_running = true;
static uint8_t screenIndex = 0;
static uint8_t fullPayoutFlag = false;

//extern uint8_t flag_ctrl_motor_provider; //Them 9/3/2025
/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi so tien tra lai co trong dau doc tien
 *
 * @param    NONE
 * @retval   NONE
 */
static void UpdateScreen()
{
    int sizeInPixel;
    GLcd_ClearScreen(BLACK);
    /* neu dau doc tien la NV11 hoac
       dau doc tien la NV200 va hien thi page thu nhat */
    if (NV11_GetUnitType() == NV_TYPE_NV11 || (NV11_GetUnitType() == NV_TYPE_NV200 && screenIndex == 0))
    {
        /* hien thi dong chu "DUA TIEN VAO" */
        sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_NOTE_FILL));
        GLcd_DrawStringUni(Lang_GetText(LANG_NOTE_FILL), (GLCD_WIDTH - sizeInPixel) / 2, 10, WHITE);
        /* lay tong so tien trong NoteFloat */
        int availableChange = NV11_GetAvailableChange();
        sprintf(strLcd, "%d", availableChange);
        /* hien thi tong so tien tra lai len LCD */
        sizeInPixel = GLcd_MeasureString(strLcd);
        GLcd_FillRect(0, 30, GLCD_WIDTH, 10, BLACK);
        GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
        GLcd_DrawString("(0)==>", 85, 55, WHITE);
    }
    else /* dau doc tien la NV200 va hien thi page thu nhat */
    {
        GLcd_SetFont(&font6x8);
        /* lay cac kenh tien te cua dau doc tien */
        int channelNumber = NV11_GetChannelNumber();
        /* */
        for (int channelIndex = 0; channelIndex < channelNumber; channelIndex++)
        {
            /* hien thi so to tien theo menh gia tien te */
            sprintf(strLcd, "%dK:%d", NV11_GetChannelValue(channelIndex) / 1000, NV11_GetStoredNoteByChannel(channelIndex));
            if (channelIndex < 3)
            {
                GLcd_DrawString(strLcd, 6, channelIndex * 13 + 3, WHITE);
            }
            else if (channelIndex < 5)
            {
                GLcd_DrawString(strLcd, 0, channelIndex * 13 + 3, WHITE);
            }
            else if (channelIndex < 6)
            {
                GLcd_DrawString(strLcd, 85, (channelIndex - 5) * 13 + 3, WHITE);
            }
            else
            {
                GLcd_DrawString(strLcd, 79, (channelIndex - 5) * 13 + 3, WHITE);
            }
        }
        GLcd_DrawString("<==(8)", 85, 55, WHITE);
        /* dua du lieu tu bo dem g_offBuffer len LCD */
        GLcd_Flush();
    }
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi menu dua tien tra lai vao may
 *
 * @param    NONE
 * @retval   NONE
 */
void SettingNoteFill_Show()
{
    Dbg_Println("Note_Fill >> Enter the setting");
    /* Initialization variable */
    int status = 0;
    int noteValue;
    uint32_t tick = GetTickCount();
    g_visible = true;
    g_running = true;
    fullPayoutFlag = false;
    /* hien thi so tien tra lai dang luu tru */
    UpdateScreen();
    /* cai dat menh gia tien mat luu tru de tra lai */
    if (NV11_GetUnitType() == NV_TYPE_NV11)
    {
        NV11_SetDenominationForChange(NV11_GetDenominationForChange());
    }
    /* dinh tuyen menh gia tien tra lai cho NV200 */
    else if (NV11_GetUnitType() == NV_TYPE_NV200)
    {
        NV11_RoutingPayoutForNV200();
    }
    /* cho phep dau doc tien hoat dong */
    NV11_Enable();
    if( NV11_GetAvailableChange() / NV11_GetDenominationForChange() >= NOTE_NUM_PAYOUT)
    {
      fullPayoutFlag = true;
    }

    while (!fullPayoutFlag)
    {
        /* neu su kien gan nhat cua dau doc tien la doc gia tri tien */
        if (NV11_GetLatestEvent() == NV11_NOTE_READ)
        {
            /* xoa toan bo xu kien*/
            NV11_ClearLatestEvent();
            /* lay menh gia tien gan nhat */
            noteValue = NV11_GetLatestNote();
            /* neu menh gia tien khac voi menh gia tien tra lai cua dau doc tien thi tra lai tien */
            if (NV11_GetUnitType() == NV_TYPE_NV11 && noteValue != NV11_GetDenominationForChange())
            {
                Dbg_Println("Note_Fill >> Note rejected. The note is not accepted");
                NV11_Reject();
            }
            else if (NV11_GetUnitType() == NV_TYPE_NV200 && NV200_ValueIsOnForPayout(noteValue) == false)
            {
                Dbg_Println("Note_Fill >> Note rejected. The note is not accepted");
                NV11_Reject();
            }
            else
            {
                /* doi dau doc tien luu tru tien tra lai */
                tick = GetTickCount();
                status = 0;
                while (NV11_GetLatestEvent() != NV11_NOTE_STORED)
                {
                    NV11_Process();
                    if(NV11_GetLatestEvent() == NV11_NOTE_STACKED || NV11_GetLatestEvent() == NV11_ERROR || \
                       NV11_GetLatestEvent() == NV11_NOTE_FRAUD_ATTEMPT)
                    {
                        status = 1;
                        break;
                    }

                    if (GetTickCount() - tick > 30000)
                    {
                        status = 1;
                        break;
                    }

                    /* Check tick all time */
                    Perh_ProcessAllWhile();
                }

                if (status == 0)
                {
                    Dbg_Println("Note_Fill >> Note filled in successfully");

                    /* Update note value */
                    UpdateScreen();
                    NV11_Enable();

                    if( NV11_GetAvailableChange() / NV11_GetDenominationForChange() >= NOTE_NUM_PAYOUT)
                    {
                      NV11_DisablePayout();
                      Dbg_Println("Note_Fill >> Tat chuc nang dua tien len PayNote == Menu ");
                      fullPayoutFlag = true;
                      break;
                    }
                }
                else
                {
                    Dbg_Println("Note_Fill >> Error to fill the note");
                    /* ve background */
                    GLcd_ClearScreen(BLACK);
                    GLcd_SetFont(&font6x8);
                    int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_STACK_STORD_NOTE_ERROR));
                    GLcd_DrawStringUni(Lang_GetText(LANG_STACK_STORD_NOTE_ERROR), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                    GLcd_Flush();
                }
            }
        }
        /* thoat khoi menu nhan tien tra lai */
        if (!g_running)
        {
            g_visible = false;
            /* hien thi lai menu */
            Setting_Show();
            /* khong cho dau doc tien hoat dong */
            NV11_Disable();
            Dbg_Println("Note_Fill >> Exit the setting");
            break;
        }
        /* cho phep xu ly ban phim de thoat khoi qua trinh nhan tien */
        Keypad_Process();
        NV11_Process();

        /* Check tick all time */
        Perh_ProcessAllWhile();
    }


    if(fullPayoutFlag)
    {
      /* hien thi day tin trong payout note trong 3s */

      g_visible = false;
      /* hien thi lai menu */
      Setting_Show();
      /* khong cho dau doc tien hoat dong */
      NV11_Disable();
      Dbg_Println("Note_Fill >> Exit the setting");
    }
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi menu
 *           lay tien tra lai
 *
 * @param    uint8_t keyCode - Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingNoteFill_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim cancel thi tra ve false  va hien thi menu */
    if (keyCode == '#')    /* CANCEL KEY */
    {
        g_running = false;
    }
    else if (keyCode == '*')    /* ENTER KEY */
    {

    }
    else if (keyCode == '0')    /* DOWN ARROW */
    {
        if (screenIndex == 0)
        {
            screenIndex = 1;
            /* cap nhat hien thi LCD */
            UpdateScreen();
        }
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        if (screenIndex == 1)
        {
            screenIndex = 0;
            /* cap nhat hien thi LCD */
            UpdateScreen();
        }
    }

    return true;
}

/**
 * @brief    Thoat menu cai dat
 *
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu
 */
bool OutSettingNoteFill()
{
     g_visible = false;
     return g_visible;
}