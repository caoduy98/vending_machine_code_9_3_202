/** @file    selling_main.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan trinh dieu khien, hien thi trong qua trinh ban hang
  */

#include "platform.h"
#include "nv11.h"
#include "glcd.h"
#include "images.h"
#include "resources.h"
#include "motor.h"
#include "adc.h"
#include "humidity.h"
#include "peripheral.h"
#include "language.h"
#include <math.h>
#include "gsm.h"
#include "wav_player.h"
#include "ftp_define.h"
#include "watchdog.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SELLING_MAIN
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
#define NV11_GATE_STATUS_PIN    PA17

#define NV11_MAIN_INTERFACE     0
#define NV11_WAIT_USER_EVENT    1

#define NV11_MESSAGE_OUT_OF_CHANGE      0
#define NV11_MESSAGE_ITEM_NOT_AVAIL     1
#define NV11_MESSAGE_OUT_OF_STOCK       2
#define NV11_MESSAGE_GIVING_CHANGE      3
#define NV11_MESSAGE_ITEM_IS_LOCKED     4


static uint8_t g_state = NV11_MAIN_INTERFACE;
int g_storedBalance = 0;
static int g_receivedNotes = 0;
int g_holdBalance = 0;


static timer_t g_tmrTimeout;
const uint8_t* g_interface;
const uint8_t* g_interface_save;
static float g_humidity, c_humidity ;
static uint32_t g_tickHumidity = 0;
uint8_t g_humidityset = 20;

/* The variable to indicate that a call is being taking place */
static bool g_gsmIsCalling = false;
static bool g_itemIsLock = false;
static uint8_t g_gsmTmp = 0;

static int GetTotalBalance();
static bool SellItem(int item);
static void DrawImage(const uint8_t* image, int x, int y);
void UpdateScreen();
static void RejectNote();
static void ShowMessage(int messageIndex, bool wait);
//static void On_tmrTimeout(timer_t* tmr, void* param);
static void KeypadHandler(key_event_t event, char ch);
static bool MotorRunService(uint8_t capicity);
void GetHumidityProcess();

extern uint8_t motorNotStop;

extern status_update_t updateFirmware;
extern uint8_t g_signalQuantity;
extern bool g_detectSimCard;
extern float g_gsmHumidityValueCheckErr;
extern bool g_isStopSellWhenEmpty;

uint32_t g_numNotePayout = 0;
uint32_t g_lastNumNotePayout = 0;
bool g_errorPayout = false;
bool g_resetNv11State = false;
uint32_t g_tickErrorDropSensor = 0;
bool isbanloi = false;

extern void CtrlMotorProviderProcess(void);
extern void CountSellAndCheckCtrlMotor(uint32_t u32cnt);
extern void CountSellAndCheckEmpty(uint32_t u32cnt);
extern void ClearSellAndCheckEmpty(void);

/**
 * @brief  Ham cap nhat lai so tien con duw va tong so tien nhan duoc tu ROM
 *
 * @param  NONE
 * @retval NONE
 */
void RestoreBalance()
{
    /* Cap nhat so tien con duw tu ROM */
    Perh_RestoreBalance(&g_storedBalance);
    /* Cap nhat tong so tien nhan duoc tu ROM */
    Perh_RestoreReceivedNotes(&g_receivedNotes);
    /* Neu so tien de tra lai > 0 */
    if (g_storedBalance > 0)
    {
        /* Trang thai tro su kien nguoi dung */
        g_state = NV11_WAIT_USER_EVENT;
    }
    else
    {
        /* Trang thai giao tiep dau doc tien */
        g_state = NV11_MAIN_INTERFACE;
    }
}
/**
 * @brief  Ham xoa toan bo so tien con duw trong ROM
 *
 * @param  NONE
 * @retval NONE
 */
void ClearBalance()
{
    g_storedBalance = 0;
    Perh_SaveBalance(g_storedBalance);
    g_state = NV11_MAIN_INTERFACE;
}
/**
 * @brief  Ham cai dat chinh, goi ban phim va man hinh ban hang
 *
 * @param  NONE
 * @retval NONE
 */
void MainSetup()
{
    Keypad_InstallCallback(KEY_PRESS, KeypadHandler);
    UpdateScreen();
}
/**
 * @brief  Ham quay lai man hinh chinh, goi ban phim va man hinh ban hang
 *
 * @param  NONE
 * @retval NONE
 */
void MainResume()
{
    Keypad_InstallCallback(KEY_PRESS, KeypadHandler);
    UpdateScreen();
}

extern uint8_t g_gsmHumidityValueMsg;   /* The variable in gsm.c file */
/**
 * @brief  Ham xu ly nghiep vu mua hang chinh
 *
 * @param  NONE
 * @retval NONE
 */
void MainProcess(void)
{
    uint32_t tick = 0;
    uint8_t status = 0;

    /* Neu co bao co yeu cau thuc hien cuoc goi */
    if (g_gsmIsCalling)
    {
        /* Neu cuoc goi da ket thuc */
        if (!Gsm_IsCalling())
        {
            /* Xoa co bao cuoc goi */
            g_gsmIsCalling = false;
            /* Xoa co bao khoa cac khay hang */
            g_itemIsLock = false;
            g_gsmTmp = 0;
            /* Cap nhat lai man hinh hien thi */
            UpdateScreen();
        }
        return;
    }
    /* Goi trinh xu ly do am */
    GetHumidityProcess();

    /* Goi trinh xu ly dau doc tien*/
    NV11_Process();

    /* Kiem tra cac trang thai may nghiep vu ban hang */
    switch (g_state)
    {
    /* Neu su kien la man hinh giao dien chinh, may dang trang thai cho */
    case NV11_MAIN_INTERFACE:
        if (updateFirmware == READY_UPDATE) __NVIC_SystemReset();
        else if(updateFirmware != READY_UPDATE)
        {
            /* Cho phep dau doc tien */
            NV11_Enable();

            /* Xoa so du moi */
            g_holdBalance = 0;
            /* Xoa tong so du */
            g_storedBalance = 0;
            /* Xoa ton bo so du luu trong ROM*/
            Perh_SaveBalance(g_storedBalance);

            /* Xoa man hinh hien thi */
            GLcd_ClearScreen(BLACK);
            /* Neu ngon ngu lua chon la tieng anh */
			if(g_isStopSellWhenEmpty) {
				GLcd_ClearScreen(BLACK);
				DrawImage(&g_out_stock, 7, 30);
				GLcd_Flush();
			}
			else {
				if (GetCurrentLanguage() == LANG_ENG)
				{
					/* Hien thi man hinh "Please select the item "*/
					DrawImage(&g_img_main_interface_en, 0, 0);
				}
				/* Neu ngon ngu lua chon la tieng viet */
				else
				{
					/* Hien thi man hinh " Moi quy khach lua chon san pham "*/
					DrawImage(&g_img_main_interface, 13, 20);
				}
			}

            /* Gui du lieu hien thi len man hinh LCD */
            GLcd_Flush();
            /* Chuyen sang trang thai tiep theo cho cac su kien, thao tac nguoi dung */
            g_state = NV11_WAIT_USER_EVENT;
        }
        break;
    /* Trang thai cho su kien va thao tac nguoi dung */
    case NV11_WAIT_USER_EVENT:

        /* Neu NV11 bao loi */
        if (NV11_GetLatestEvent() == NV11_ERROR)
        {
            Dbg_Println("Selling_main >> NV11_ERROR >> BREAK");
            GLcd_ClearScreen(BLACK);
            DrawImage(&NV11_error, 0, 0);
            GLcd_Flush();
            break;      /* Thoat */
        }
        else if (g_resetNv11State == true && NV11_GetLatestEvent() != NV11_ERROR)
        {
            Dbg_Println("Selling_main >> Clear display error");
            //g_holdBalance = 0;
            g_resetNv11State = false;
            UpdateScreen();
        }
        /* Neu NV11 bao tien vua dua ra khoi cua dau doc tien */
        else if (NV11_GetLatestEvent() == NV11_NOTE_REJECTED)
        {
            /* Xoa xu kien */
            NV11_ClearLatestEvent();
            /* Xoa so du dang giu */
            g_holdBalance = 0;
            /* Cap nhat lai hien thi */
            UpdateScreen();
        }
        /* Neu dau doc tien bao su kien NV11 vua nhan 1 to tien */
        else if (NV11_GetLatestEvent() == NV11_NOTE_READ)
        {
            Dbg_Println("Selling_main >> NV11 is Received one Note");
            /* Xoa su kien */
            NV11_ClearLatestEvent();
            {
                Dbg_Println("Selling_main >> Item Is Just Selected.");
                /* Tien hanh nhan tien vao hop dung tien va xu ly */
                tick = GetTickCount();
                /* Xoa co bao loi */
                status = 0;

                /* lon hon so to tien thi khong nhan them tien */
                if (NV11_GetLatestNote() == NV11_GetDenominationForChange() && NV11_GetUnitType() == NV_TYPE_NV11)
                {
                    if( NV11_GetAvailableChange() / NV11_GetDenominationForChange() >= NOTE_NUM_PAYOUT)
                    {
                        NV11_DisablePayout();
                        Dbg_Println("Selling_main >> Tat chuc nang dua tien len PayNote");
                    }
                    else
                    {
                        NV11_EnablePayout();
                        Dbg_Println("Selling_main >> Bat chuc nang dua tien len PayNote");
                    }
                }


                /* Cho su kien NV1 bao da luu tru tien thanh cong*/
                while (NV11_GetLatestEvent() != NV11_NOTE_STACKED && NV11_GetLatestEvent() != NV11_NOTE_STORED)
                {
                    /* Trinh xu ly su kien NV11 */
                    NV11_Process();
                    if((NV11_GetLatestEvent() == NV11_NOTE_STACKED) || (NV11_GetLatestEvent() == NV11_NOTE_STORED))
                    {
                        Dbg_Println("Selling_main >> Stored Note success");
                        break;
                    }
                    /* Neu sau 12s hoac NV11 bao tien da duoc dua ra khoi may */
                    if ((GetTickCount() - tick > 30000) || (NV11_GetLatestEvent() == NV11_NOTE_REJECTED) || (NV11_GetLatestEvent() == NV11_NOTE_FRAUD_ATTEMPT))
                    {
                        /* set co bao loi*/
                        status = 1;
                        Dbg_Println("Selling_main >> Stored Note Error");
                        break;  /* Thoat */
                    }
                    /* Check tick all time */
                    Perh_ProcessAllWhile();
                }
                /* Neu co bao khong co loi trong qua trinh luu tru tien */
                if (status == 0)
                {
                    /* Dung dau doc tien */
                    NV11_Disable();
                    /* Cong them so tien vua nhan vao tong so tien dua vao may */
                    Perh_AddNoteIn(NV11_GetLatestNote());
                    /* Tinh lai tong so duw = cong them menh gia tien vua nhan */
                    g_storedBalance += NV11_GetLatestNote();
                    /* Tinh lai tong so tien vua nhan = cong them menh gia tien vua nhan */
                    g_receivedNotes += NV11_GetLatestNote();
                    /* Luu so duw vao ROM*/
                    Perh_SaveBalance(g_storedBalance);
                    /* Luu tong tien da nhan vao ROM */
                    Perh_SaveReceivedNotes(g_receivedNotes);
                    /* Cap nhat lai cac gia tri len man hinh */
                    sprintf(bufferDebug,"Selling_main >> SumBalance: %8dVND, SumNoteRecerved: %8dVND.",g_storedBalance,g_receivedNotes);
                    Dbg_Println(bufferDebug);

                    UpdateScreen();

                    /* Thuc hien chuong trinh ban mot san pham */
                    if (SellItem(1))
                    {
                        Dbg_Println("Selling_main >> Selling Item is Success.");
                        /* Cho den khi mua hang thanh cong, kiem tra lai so du */
                        if (GetTotalBalance() == 0)
                        {
                            Dbg_Println("Selling_main >> Balance: 0VND");
                            /* Neu so du = 0, quay lai trang thai man hinh chinh -> may trong trang thai cho */
                            g_state = NV11_MAIN_INTERFACE;
                        }
                    }
                    else
                    {

                    }
                }
                else if(status == 1)
                {
                    Dbg_Println("Selling_main >> Stored Note Error -- RejectNote1");
                    /* Xoa xu kien */
                    NV11_ClearLatestEvent();
                    /* Xoa so du dang giu */
                    g_holdBalance = 0;
                    /* Cap nhat lai hien thi */
                    UpdateScreen();
                }
                /* Chay lai dau doc tien */
                NV11_Enable();

            }
        }

        uint32_t g_timeoutTick = GetTickCount();
        /* Sau moi 1s cap nhat hien thi thoi gian, do am khoang hang */
        if (g_timeoutTick - g_tickHumidity > 1000)
        {
            /* Display current time on the screen */
            if ((g_interface == g_img_main_interface) || (g_interface == g_img_main_interface_en))
            {
                int second, minute, hour;
                Rtc_GetCurrentTime(&hour, &minute, &second);
                sprintf(strLcd, "%02d:%02d", hour % 24, minute % 60);
                GLcd_FillRect(0, 3, GLCD_WIDTH, 8, BLACK);
                GLcd_SetFont(&font6x8);
                GLcd_DrawString(strLcd, 45, 3, WHITE);
            }

            /* hien thi gia tri do am */
            sprintf(strLcd,"%02.0f(%)", c_humidity);
            g_tickHumidity = g_timeoutTick;

            if ((g_interface == g_img_main_interface) || (g_interface == g_img_main_interface_en))
            {
                /* hien thi trang thai song */
                GLcd_SetFont(&font6x8);
                g_interface_save = g_interface;
                GLcd_DrawString(strLcd, 87, 3, WHITE);

                if(g_detectSimCard == true)
                {
                    DrawImage(&g_img_signal_sim, 3, 2);

                    /* Hien thi tin hieu sim */
                    if(g_signalQuantity >= 20)
                    {
                        GLcd_FillRect(17, 3, 2 , 7 , WHITE);
                    }
                    if(g_signalQuantity >= 15)
                    {
                        GLcd_FillRect(14, 5, 2 , 5 , WHITE);
                    }
                    if(g_signalQuantity >= 10)
                    {
                        GLcd_FillRect(11, 7, 2 , 3 , WHITE);
                    }
                    GLcd_Flush();
                }
                else
                {
                    /* hien thi khong co sim */
                    DrawImage(&g_img_not_detect_sim, 3, 2);
                    GLcd_Flush();
                }
                g_interface = g_interface_save;
            }
            Control_Humidity(g_humidity, g_humidityset);
        }
        break;
    }
}
/**
 * @brief  Ham xu ly dieu khien do am
 *
 * @param  NONE
 * @retval NONE
 */
void GetHumidityProcess()
{
    static uint32_t g_tick = 0;
    /* Update sau moi 1s */
    if (GetTickCount() - g_tick > 3000)
    {
        g_tick = GetTickCount();
        float temp = 0;
        if(Read_Humidity(&temp)) {
            c_humidity = temp;
        }
        g_humidity = c_humidity;
        g_gsmHumidityValueCheckErr = c_humidity;
        g_gsmHumidityValueMsg = (uint8_t)c_humidity;
    }
}


/**
 * @brief  Ham tra ve tong so duw (bang so duw cu + menh gia tien gan nhat dua vao hop dung tien)
 *
 * @param  NONE
 * @retval NONE
 */
static int GetTotalBalance()
{
    return g_storedBalance + g_holdBalance;
}

static void DrawImage(const uint8_t* image, int x, int y)
{
    g_interface = image;
    GLcd_DrawBitmap(g_interface, x, y);
}
/**
 * @brief  Ham xu ly cap nhan lai man hinh hien thi
 *
 * @param  NONE
 * @retval NONE
 */
void UpdateScreen()
{
    /* Neu khong co san pham duoc chon va tong so du bang 0*/
    if (GetTotalBalance() == 0)
    {
        /* Dat dang thai may ve trang thai man hinh chinh */
        g_state = NV11_MAIN_INTERFACE;
        return;
    }
    /* Xoa man hinh */
    GLcd_ClearScreen(BLACK);
	if(g_isStopSellWhenEmpty) {
		GLcd_ClearScreen(BLACK);
		DrawImage(&g_out_stock, 7, 30);
		GLcd_Flush();
	}else {
		/* Neu ngon ngu tieng anh */
		if (GetCurrentLanguage() == LANG_ENG)
		{
			DrawImage(&g_interface_1_en, 0, 0);
		}
		/* Neu ngon ngu tieng viet */
		else
		{
			DrawImage(&g_interface_1, 0, 0);
		}
		/* Hien thi gia tri cua so du hien tai */
		sprintf(strLcd, "%04d", GetTotalBalance());
		GLcd_SetFont(&font6x8);
		GLcd_DrawString(strLcd, 83, 9, WHITE);
	}

    /* Gui buffer du lieu toi man hinh LCD */
    GLcd_Flush();
}
/**
 * @brief  Ham xu ly tra tien dang giu o cua dau doc cho khach hang
 *
 * @param  NONE
 * @retval NONE
 */
static void RejectNote()
{
    NV11_Reject();
    g_holdBalance = 0;
    UpdateScreen();
}

/**
 * @brief  Ham xu ly hien thi thong tin thong diep len man hinh
 *
 * @param  int messageIndex -> ID cua thong diep can hien thi
 * @param  bool wait -> bit lua chon tinh nang tre 3s chay lai NV11
 * @retval NONE
 */
static void ShowMessage(int messageIndex, bool wait)
{
    uint32_t tick = 0;
    bool holdNoteFlag = false;
    switch (messageIndex)
    {
    /* Neu ID thong diep "HET TIEN TRA LAI"*/
    case NV11_MESSAGE_OUT_OF_CHANGE:
        Wav_Stop();
        Wav_Play("VM_009.wav");
        /* Xoa man hinh*/
        GLcd_ClearScreen(BLACK);
        /* Neu ngon ngu cai dat la tieng Anh */
        if (GetCurrentLanguage() == LANG_ENG)
        {
            /* Lay mang graphic chuoi ki tu tieng anh hien thi */
            DrawImage(&g_interface_out_of_change_en, 5, 16);
        }
         /* Neu ngon ngu cai dat la tieng Viet */
        else
        {
            /* Lay mang graphic chuoi ki tu tieng viet hien thi */
            DrawImage(&g_interface_out_of_change, 5, 16);
        }
        break;
    /* ID thong diep "HANG KHONG TON TAI "*/
    case NV11_MESSAGE_ITEM_NOT_AVAIL:
        Wav_Stop();
        Wav_Play("VM_001.wav");
        GLcd_ClearScreen(BLACK);
        if (GetCurrentLanguage() == LANG_ENG)
        {
            DrawImage(&g_interface_item_not_avail_en, 16, 13);
        }
        else
        {
            DrawImage(&g_interface_item_not_avail, 16, 13);
        }
        break;
    /* ID thong dien "SAN PHAM HET HANG "*/
    case NV11_MESSAGE_OUT_OF_STOCK:
        Wav_Stop();
        Wav_Play("VM_002.wav");
        GLcd_ClearScreen(BLACK);
        if (GetCurrentLanguage() == LANG_ENG)
        {
            DrawImage(&g_interface_out_of_stock_en, 26, 12);
        }
        else
        {
            DrawImage(&g_interface_out_of_stock, 26, 12);
        }
        break;
    /*  ID thong diep "MAY DANG TRA LAI TIEN "*/
    case NV11_MESSAGE_GIVING_CHANGE:
        Wav_Stop();
        Wav_Play("VM_008.wav");
        GLcd_ClearScreen(BLACK);
        if (GetCurrentLanguage() == LANG_ENG)
        {
            DrawImage(&g_interface_giving_change_en, 3, 22);
        }
        else
        {
            DrawImage(&g_interface_giving_change, 3, 22);
        }
        break;
    /* ID thong diep "SAN PHAM BI LOI"*/
    case NV11_MESSAGE_ITEM_IS_LOCKED:
        Wav_Stop();
        Wav_Play("VM_007.wav");
        GLcd_ClearScreen(BLACK);
        if (GetCurrentLanguage() == LANG_ENG)
        {
            DrawImage(&g_img_item_error_en, 11, 23);
        }
        else
        {
            DrawImage(&g_img_item_error, 11, 23);
        }
        break;
    }
    /* Gui mang du lieu len LCD*/
    GLcd_Flush();
    /* Neu bit wait duoc set */
    if (wait)
    {
        /* Tat dau doc tien */
        NV11_Disable();
        /* Tre 3s */
        tick = GetTickCount();
        while(GetTickCount() - tick < 3000)
        {
            NV11_Process();
            if (messageIndex != NV11_MESSAGE_GIVING_CHANGE && holdNoteFlag == false && NV11_GetLatestEvent() == NV11_NOTE_READ)
            {
                NV11_Hold();
                holdNoteFlag = true;
            }
            else if(messageIndex == NV11_MESSAGE_GIVING_CHANGE && NV11_GetLatestEvent() == NV11_NOTE_READ)
            {
                NV11_Reject();
            }
            /* Check tick all time */
            Perh_ProcessAllWhile();
        }
        /* Cap nhat lai man hinh */
        UpdateScreen();
        /* Bat dau doc tien */
        NV11_Enable();

    }
}
/**
 * @brief  Ham soft time tre thoi gian sau 3s nhan san pham khach lua chon dua xu ly
 *
 * @param  timer_t* tmr -> Con tro den dia chi timer
 * @param  void* param -> tham so can truyen vao ham callback
 * @retval NONE
 */

/**
 * @brief  Ham xu ly xu kien ban phim
 *
 * @param  key_event_t event -> Con tro den xu kien ban phim
 * @param  char ch -> Phim vua an
 * @retval NONE
 */
static void KeypadHandler(key_event_t event, char ch)
{
    /* Neu dang thuc hien 1 cuoc goi */
    if (g_gsmIsCalling)
    {
        /* Neu nhan phim '#' -> Huy cuoc goi */
        if (ch == '#')
        {
            /* Neu cuoc goi dang dien ra -> thuc hien gach may ket thuc cuoc goi */
            Gsm_HangCall();
            /* Xoa co bao khay hang bi khoa */
            g_itemIsLock = false;
            /* Xoa co bao dang thuc hien cuoc goi */
            g_gsmIsCalling = false;
            g_gsmTmp = 0;
            UpdateScreen();
        }
        return;     /* Cac phim khac khong duoc nhan trong qua trinh thuc hien cuoc goi */
    }
    /* Neu xu kien NV11 tra ve dang bi loi */
    if (NV11_GetLatestEvent() == NV11_ERROR)
    {
        return;         /* Thoat */
    }
    /* Neu su kien ban phim la nhan phim */
    if (event == KEY_PRESS)
    {
        /* Phim duoc nhan la phim Enter */
        if (ch == '*')
        {
        }
        /* Neu phim duoc nhan la phin Cancel */
        else if (ch == '#')
        {

        }
        /* Neu la cac phim khac */
        else
        {
            /* Neu khay san pham dang khoa va phim duoc an la phim "0" */
            if (g_itemIsLock && ch == '0')
            {
                /* Thuc hien nhan 2 lan phim 0 de thuc hien 1 cuoc goi bao loi toi tong dai */
                if (g_gsmTmp == 0)      /* Nhan lan 1 */
                {
                    g_gsmTmp = 1;
                }
                else                    /* Nhan lan 2 */
                {
                    g_gsmTmp = 0;
                    /* Dung timer */
                    Tmr_Stop(&g_tmrTimeout);
                    /* Thuc hien mot cuoc goi */
                    Gsm_MakeCall();
                    /* Bat co bao thuc hien cuoc goi */
                    g_gsmIsCalling = true;
                }
            }
        }
    }
}
/**
 * @brief  Ham xu ly ban hang
 *
 * @param  int itemId -> Vi tri khay chua hang
 * @retval NONE
 */
uint8_t last_is_error = 0;
uint8_t s_isNotRunMotorWhenEmpty = 0;
static bool SellItem(int itemId)
{
    /* Neu so duw > 0 va so duw > so tien cua hang hoa dang chon */
    // Kiem tra menh gia tien
    uint16_t s_selectedNote = Perh_GetAcceptableNoteHight();
    s_selectedNote <<= 8;
    s_selectedNote |= Perh_GetAcceptableNoteLow();
    uint8_t s_check_note_band = false;

    /* bit 8 | bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 */
    /* 500K  | 200k  |  100k |   50k |  20k  |   10k |   5k  |   2k  |   1k  */
    uint32_t s_note[9] = {1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000};
    for (int i = 0; i < 9; ++i)
    {
        if ((((s_selectedNote >> i) & 0x01) == 0x00) && (s_note[i] == g_storedBalance))
        {
            s_check_note_band = true;
            break;
        }
    }

    if (!s_check_note_band)
    {
        /* Dung dau doc tien */
        Dbg_Println("Selling_main >> Balance > Itemprice: Process SellItem.");
        NV11_Disable();
        s_isNotRunMotorWhenEmpty = 0;
        /* Thuc hien trinh dieu khien dong co quay tra san pham */
        if (MotorRunService(Perh_GetItemNumber(1)))    /* Neu tra san pham ok */
        {
            CountSellAndCheckCtrlMotor(Perh_GetItemNumber(1));
            Dbg_Println("Selling_main >> Run Motor: OK");
            /* Gui thong tin tong so tien da nhan duoc va ma hang da ban len server */
            SendReleasedItemFrame(g_receivedNotes / 1000, itemId);
            
            isbanloi = 0;           
            //flag_ctrl_motor_provider = 0; //Them 9/3/2025
            //flag_ctrl_motor_provider_swap = 0;

//            if(last_is_error == 1) {
//                last_is_error = 0;
//                SendMotorResumeFrame();
//            }
            SendMotorResumeFrame();
            sprintf(bufferDebug,"Selling_main >> SendSever:SumNote %8dVND, ItemID:%4d",g_receivedNotes,itemId);
            Dbg_Println(bufferDebug);

            /* Xoa tong so tien da nhan duoc*/
            g_receivedNotes = 0;
            /* Xoa du lieu tong so da nhan duoc trong ROM */
            Perh_SaveReceivedNotes(0);
            /* Cap nhat lai so du: So du moi = so du cu - menh gia hang da ban */
            // g_storedBalance -= Perh_GetItemPrice(itemId);
            g_storedBalance = 0;
            sprintf(bufferDebug,"Selling_main >> SumBalance: %8dVND",g_storedBalance);
            Dbg_Println(bufferDebug);

            /* Ghi so du vao ROM */
            Perh_SaveBalance(g_storedBalance);
            /* Luu cac thong tin ve doanh so vao the nho va ROM -> luu so tien hang vua ban */
            Resource_SaveSalestoEeprom(Perh_GetItemPrice(itemId));
            sprintf(bufferDebug,"Selling_main >> Sale to Eeprom: +%8dVND",Perh_GetItemPrice(itemId));
            Dbg_Println(bufferDebug);

            Wav_Stop();
            Wav_Play("VM_006.wav");
            /* Cap nhat lai man hinh hien thi */
            if(!s_isNotRunMotorWhenEmpty) {
                GLcd_ClearScreen(BLACK);
                DrawImage(&g_interface_thankyou, 0, 0);
                GLcd_Flush();

                Delay(500);
                Perh_ProcessAllWhile();
                Delay(500);
                Perh_ProcessAllWhile();
                Delay(500);
                Perh_ProcessAllWhile();
                Delay(500);
                Perh_ProcessAllWhile();GLcd_ClearScreen(BLACK);
                DrawImage(&g_interface_thankyou, 0, 0);
                GLcd_Flush();

                Delay(500);
                Perh_ProcessAllWhile();
                Delay(500);
                Perh_ProcessAllWhile();
                Delay(500);
                Perh_ProcessAllWhile();
                Delay(500);
                Perh_ProcessAllWhile();
            }


            UpdateScreen();
            /* Xoa co ban khoa khay hang */
            g_itemIsLock = false;
            /* Bat lai dau doc tien */
            NV11_Enable();
            /* Tra ve trang thai mua hang thanh cong */
            return true;
        }
        /* Neu motor bao loi, hoac cam bien roi loi */
        else
        {
            isbanloi = 1;
            //flag_ctrl_motor_provider = 0; //Them 9/3/2025
            //flag_ctrl_motor_provider_swap = 0;
            NV11_Disable(); // Them 24/2/2025
            
            last_is_error = 1;
            Dbg_Println("Selling_main >> Run Motor: ERROR");
            /* Gui frame mua hang loi len server: Bao gom tong so tien nhan va khay hang*/
            SendSellingErrorFrame(g_receivedNotes / 1000, itemId);
            /* Xoa tong gia tri tien nhan */
            g_receivedNotes = 0;
            /* Gui frame bao loi len server */
            SendMotorErrorFrame(itemId);
            /* Xoa du lieu tong so da nhan duoc trong ROM */
            Perh_SaveReceivedNotes(0);
            /* Cap nhat lai so du: So du moi = so du cu - menh gia hang da ban */
            // g_storedBalance -= Perh_GetItemPrice(itemId);
            g_storedBalance = 0;
            sprintf(bufferDebug,"Selling_main >> SumBalance: %8dVND",g_storedBalance);
            Dbg_Println(bufferDebug);
            /* Ghi so du vao ROM */
            Perh_SaveBalance(g_storedBalance);
            GLcd_ClearScreen(BLACK);
            DrawImage(&g_out_stock, 7, 30);
            GLcd_Flush();
            Delay(500);
            Perh_ProcessAllWhile();
            Delay(500);
            Perh_ProcessAllWhile();
            Delay(500);
            Perh_ProcessAllWhile();
            Delay(500);
            Perh_ProcessAllWhile();
            UpdateScreen();
        }
        /* Chay lai dau doc tien */
        NV11_Enable();

        return false;   /* Bao trang thai ban hang bi loi*/
    }

    /* Neu so duw < 0 hoac so duw < menh gia hang lua chon*/
    else
    {
        NV11_Enable();
        /* Cho phep dau doc tien */
    }
    return false;       /* Bao trang thai ban hang bi loi*/
}
/**
 * @brief  Ham xu ly tra lai tien cho nguoi dung neu la dau doc NV200
 *
 * @param  NONE
 * @retval NONE
 */
void PayoutNV200()
{
#define PAYOUT_PREPARE      0   /* Trang thai tinh so tien tra lai*/
#define PAYOUT_START        1   /* Trang thai tra lai tien */
#define PAYOUT_PROCESS      2   /* Trang thai update so du va thuc hien lai neu con */

    uint8_t state = 0;          /* Trang thai may */
    uint32_t valueToPayout;     /* So tien phai tra */
    uint32_t refundNote;        /* So tien hien tai con lai trong float note */
    uint32_t currentBalance;    /* So du hien tai */
    uint32_t holdingNoteValue;  /* Trang thai giu tien */

    state = PAYOUT_PREPARE;

    while (1)
    {
        /* Trang thai 1, chuan bi tien de tra */
        switch (state)
        {
        case PAYOUT_PREPARE:
            /* Lay tong so tien thuc te co the tra bang NV200 trong Float note*/
            refundNote = NV11_GetAvailableChange();
            /* Lay tong so du phai tra */
            currentBalance = GetTotalBalance();
            /* Lay trang thai may dang giu tien hay khong*/
            holdingNoteValue = NV11_IsHolding();
            /* Neu so du = 0, thoat */
            if (currentBalance == 0)
            {
                return;
            }
            /* Neu so tien thuc te trong float note = 0*/
            else if (refundNote == 0)
            {
                /* Hien thi "OUT OF CHANGE" va thoat */
                ShowMessage(NV11_MESSAGE_OUT_OF_CHANGE, true);
                return;
            }
            /* Neu dang la su kien giu tien o cua dau doc */
            else if (holdingNoteValue > 0)
            {
                /* Tra lai tien giu o dau doc cho nguoi dung */
                RejectNote();
                return;
            }
            /* Neu cai dat khong cho phep dau doc tra lai tien */
            else if (!Perh_RefundIsOn())
            {
                /* Hien thi "OUT OF CHANGE" va thoat */
                ShowMessage(NV11_MESSAGE_OUT_OF_CHANGE, true);
                return;
            }
            /* Neu gia tri tien thuc te con lai < so du phai tra */
            else if (refundNote < currentBalance)
            {
                /* Nhan so tien tra cho khach = tong so tien con lai trong float note */
                valueToPayout = refundNote;
            }
            /* Neu gia tri tien thuc te con lai >= so du phai tra */
            else
            {
                /* Nhan so tien tra cho khach = so du hien tai */
                valueToPayout = currentBalance;
            }
            /* Chuyen sang trang thai bat dau tra */
            state = PAYOUT_START;
            break;
        /* Chuyen sang trang thai bat dau tra */
        case PAYOUT_START:
            /* Thuc hien ham tra lai tien, gia tri tra lai 0 = ok, 1 = not ok*/
            if (NV11_PayoutValue(valueToPayout) == 0)
            {
                /* Neu ok -> Hien thi "GIVING THE CHANGE" chuyen toi trang thai PAYOUT_PROCESS */
                ShowMessage(NV11_MESSAGE_GIVING_CHANGE, false);
                state = PAYOUT_PROCESS;
            }
            else
            {
                /* Neu khong ok -> Hien thi "NOT ENOUGH MONEY TO PAYOUT" va thoat */
                ShowMessage(NV11_MESSAGE_OUT_OF_CHANGE, true);
                return;
            }
            break;

        case PAYOUT_PROCESS:
            /* Trinh xu ly xu kien tra lai cua NV11*/
            NV11_Process();
            /* Neu su kien NV200 bao ve thanh toan loi */
            if (NV11_GetLatestEvent() == NV200_PAYOUT_FAILED)
            {
                /* Tre 5s va thuc hien lai chu trinh tra tien duw tu dau */
                Delay(5000);
                state = PAYOUT_PREPARE;
                break;
            }
            /* Neu su kien cung cung bao thanh toan xong */
            else
            {
                if (NV11_GetLatestEvent() == NV11_NOTE_DISPENSED)
                {
                    /* Tinh lai so tien da tra cho khach: = tong so tien cu - tong so tien moi trong float note */
                    uint32_t paidoutValue = refundNote - NV11_GetAvailableChange();
                    /* Cong them so tien vua tra vao tong so tien da xuat ra*/
                    Perh_AddNoteOut(paidoutValue);
                    /* Tinh la so tien duw cua khach */
                    g_storedBalance -= paidoutValue;
                    /* Luu so tien du vao ROM */
                    Perh_SaveBalance(g_storedBalance);
                    /* Neu so tien du bang 0 */
                    if (g_storedBalance == 0)
                    {
                        UpdateScreen();
                        return;
                    }
                    /* Neu so du khac 0 va so tien trong float note = 0*/
                    else if (NV11_GetAvailableChange() == 0)
                    {
                        ShowMessage(NV11_MESSAGE_OUT_OF_CHANGE, true);
                        UpdateScreen();
                        return;
                    }
                    /* Neu so du khac 0 va so tien trong float khac 0 */
                    else
                    {
                        UpdateScreen();
                        ShowMessage(NV11_MESSAGE_GIVING_CHANGE, false);
                    }
                }
            }
            break;
        }

        /* Check tick all time */
        Perh_ProcessAllWhile();
    }
}

/**
 * @brief  Ham xu ly dieu khien dong co quay tra 01 san pham
 *
 * @param  uint8_t capicity -> So luong que nhang can ban
 * @retval bool -> Trang thai tra san pham(true = ok, false = loi)
 */

static bool MotorRunService(uint8_t capicity)
{
    uint32_t tick = 0;
    uint32_t count_item = 0;

    if(g_isStopSellWhenEmpty) {
        s_isNotRunMotorWhenEmpty = 1;
        return true;
    }

    /* Dung dau doc tien NV11 */
    NV11_Disable();

    /* Chay dong co lua chon theo ID */
    Perh_ProcessAllWhile();
    motorNotStop = MOTOR_NOT_STOP_FLAG_CLEAR;
    Perh_ProcessAllWhile();
//    ClearSellAndCheckEmpty();

    // Step 1: Motor run
    MotorRun();
    
    uint8_t flag_ctrl_motor_provider = 0; 
    uint8_t flag_ctrl_motor_provider_swap = 0;

    while(count_item != capicity) {

        /* Xoa co bao vat da roi */
        Perh_ClearDropSensorFlag(); 
        CtrlMotorProviderProcess();
        
        tick = GetTickCount();

        // Step 2: Doi tin hieu cam bien vi tri neu khong thi la loi -> hien thi thong bao
         while(!Perh_ItemIsDetected() && ((GetTickCount() - tick) < 30000)) { 
            Wdog_Refesh();
            
            Perh_CheckDropSensorProcess(); 
            CtrlMotorProviderProcess();
            
            
              if((GetTickCount() - tick) >= 10000) { 
                 //Dieu khien dong co cap
                switch (flag_ctrl_motor_provider)
                {
                    case 0:
                        MotorProvideRunSwap(); // Them 4/3/2025
                        MotorProvideRun();
                        flag_ctrl_motor_provider = 1;
                        break;
                    
                    case 1:
                            if (((GetTickCount() - tick) >= 12000)) { 
                            MotorProvideStop();
                            MotorProvideStopSwap(); // Them 4/3/2025
                            flag_ctrl_motor_provider = 2;
                        }
                        break;
                        
                    default:
                    break;
                }
              }
                          
              if((GetTickCount() - tick) >= 12500) {   
                   switch (flag_ctrl_motor_provider_swap)
                {
                        
                    case 0:
                            MotorProvideStopSwap(); // Them 4/3/2025
                            MotorProvideRun();
                            flag_ctrl_motor_provider_swap = 1;
  
                        break;
                        
                    case 1:
                          if (((GetTickCount() - tick) >= 14500)) {
                            MotorProvideStop();
                            MotorProvideRunSwap();
                            flag_ctrl_motor_provider_swap = 2;
                        }
                        break;     
                        
                        default:
                        break;
                  }
                }  
        }
        MotorProvideStop(); 
        if(Perh_ItemIsDetected()) {
            count_item++;
            flag_ctrl_motor_provider = 0; //Them 9/3/2025
            flag_ctrl_motor_provider_swap = 0;

            if(SensorOurStock_IsEmpty()) { 
                CountSellAndCheckEmpty(1);
            }
            
            if(count_item == capicity) {
                // Step 3: Motor stop
                MotorStop();
                Trapdoor_Open();
                Delay(200);
                Perh_ProcessAllWhile();
                Trapdoor_Off();
                return true;
            }
        } else {
            MotorStop();
            return false;
        }
        
        if ((GetTickCount() - tick) > 30000) { 
            MotorStop();
            return false;
        }
    }
    MotorStop();
    return false;
}

void updateCountDownReject(uint8_t value)
{
  sprintf(strLcd, "%02d", value);
  GLcd_FillRect(0, 3, 30, 12, BLACK);
  GLcd_SetFont(&font16x12);
  GLcd_DrawString(strLcd, 45, 3, WHITE);
  GLcd_Flush();
}

void updateCountDownPayout(uint8_t value)
{
  sprintf(strLcd, "%02d", value);
  GLcd_FillRect(45, 3, 30, 12, BLACK);
  GLcd_SetFont(&font16x12);
  GLcd_DrawString(strLcd, 45, 3, WHITE);
  GLcd_Flush();
}
