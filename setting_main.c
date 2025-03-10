/** @file    setting_main.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi cai dat tham so cho may ban hang
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "listbox.h"
#include "ff.h"
#include "glcd.h"
#include "setting_id.h"
#include "nv11.h"
#include "language.h"
#include "peripheral.h"
#include "gsm.h"
#include "eeprom.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_SETTING_MAIN
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void ClearBalance();
extern bool Door_IsOpen();
extern bool OutSettingSlot();
extern bool OutSettingPrice();
extern bool OutSettingTime();
extern bool OutSettingEmptyNote();
extern bool OutSettingNoteFill();
extern bool OutSettingOnOff();
extern bool OutSettingViewBillChange();
extern bool OutSettingViewId();
extern bool OutSettingViewSales();
extern bool OutSettingHumidity();
extern bool OutSettingViewError();
extern bool OutSettingViewNoteInOut();
extern bool OutSettingAcceptNote();
extern bool OutSettingRefundNote();
extern bool OutSettingOperatorNumber();
extern void SendRtcResumeFrame();
extern void SendEepromResumeFrame();
extern void SendSDCardResumeFrame();
extern void GetHumidityProcess();
extern bool OutSettingViewFirmwareVer();
extern bool OutSettingInitParam();
extern void RestoreEepBufferPointer();
extern bool OutSettingPassWord();
extern bool OutSettingTestSlot();
extern bool OutSettingN1Time();
extern bool OutSettingN2Capacity();
extern bool OutSettingN3Capacity();
extern bool OutSettingClearErrorSell();
//extern bool OutSettingClearSales();
/*********************************************************************************
 * DEFINE
 */

#define MACHINE_MENU       0x01u
#define TECHNICAL_MENU     0x02u
#define ADMIN_MENU         0x04u

/*********************************************************************************
 * TYPEDEF
 */

/**
 *  @brief cau truc du lieu cua mot dong hien thi tren LCD
 */
typedef struct
{
    uint32_t id;                        /*!< ID cua thanh phan hien thi menu chinh            */
    language_id_t textId;               /*!< vi tri cua text hien thi trong g_languageStrings */
    void* subItems;                     /*!< Con tro den du lieu hien thi sub menu            */
} menu_item_t;

/*********************************************************************************
 * STATIC VARIABLE
 */
static uint8_t g_interfaceIndex = 0;
static uint8_t g_mainMenuId = 0;
static listbox_t g_mainListBox;
static listbox_t g_subListBox;
static uint8_t g_setPassIndex = 0;
static uint8_t g_rankAccess = 0;
bool gPasswordInTrue = false;
static bool g_cancelIsRequested = false;

static menu_item_t g_sub_menu_sales[] =
{
    {0, LANG_DAYLY_SALES, NULL },
    {0, LANG_MONTHLY_SALES, NULL },
    {0, LANG_YEARLY_SALES, NULL },
    {0, LANG_TOTAL_SALES, NULL },
    {ID_ENDING, NULL, NULL }
};

static menu_item_t g_sub_menu_language[] =
{
    {0, LANG_VIETNAMESE, NULL },
    {0, LANG_ENGLISH, NULL },
    {ID_ENDING, NULL, NULL }
};

static menu_item_t g_sub_menu_error_log[] =
{
    {0, LANG_MOTOR_ERROR, NULL },
    {0, LANG_PAYMENT_SYSERR, NULL },
    {0, LANG_DROP_SENSOR_ERROR, NULL },
    {0, LANG_CLEAR_ERROR, NULL },
    {0, LANG_CLEAR_ERROR_HW, NULL },
    {0, LANG_CLEAR_ERROR_DROP_SENSOR, NULL},
    {ID_ENDING, NULL, NULL }
};

static menu_item_t g_sub_menu_audio_setting[] =
{
    {0, LANG_AUDIO_ONOFF, NULL },
    {0, LANG_MAIN_VOLUME, NULL },
    {0, LANG_CALL_VOLUME, NULL },
    {0, LANG_MICROPHONE_LEVEL, NULL },
    {ID_ENDING, NULL, NULL }
};


static menu_item_t g_mainMenuItems[] =
{
  /* Machine Operator  +  Technical staff */
  {ID_TOTAL_SALES, LANG_TOTAL_SALES, &g_sub_menu_sales },                   /* 0 */
  {ID_SET_CAPACITY, LANG_PRICE_CAPACITY, NULL },                            /* 1 */
  {ID_ERROR_LOG, LANG_ERROR_LOG, &g_sub_menu_error_log },                   /* 2 */
  {ID_HUMIDITY_ONOFF, LANG_HUMIDITY_ONOFF, NULL },                          /* 3 */
  {ID_SET_HUMIDITY, LANG_SET_HUMIDITY, NULL },                              /* 4 */
  {ID_ACCEPT_NOTE, LANG_ACCEPT_NOTE, NULL },                                /* 5 */

  {ID_N1_TIME, LANG_N1_TIME, NULL},
  {ID_N2_CAPACITY, LANG_N2_CAPACITY, NULL},
  {ID_N3_CAPACITY, LANG_N3_CAPACITY, NULL},
  {ID_CLEAR_ERROR_SALE, LANG_CLEAR_SELL, NULL},


  {ID_LANGUAGE, LANG_LANGUAGE, &g_sub_menu_language },                      /* 6 */
  {ID_CHANGE_PASSWORD, LANG_CHANGE_PASSWORD, NULL },                        /* 7 */
  {ID_VIEW_FIRMWARE, LANG_VIEW_FIRMWARE, NULL},                             /* 8 */
  /* Technical staff */
  {ID_AUDIO_SETTING, LANG_AUDIO_SETTING, &g_sub_menu_audio_setting },       /* 9 */
  {ID_SET_TIME, LANG_SET_TIME, NULL },                                      /* 10 */
  {ID_OPERATOR_NUMBER, LANG_OPERATOR_NUMBER, NULL },                        /* 11 */
  {ID_VIEW_VEND_ID, LANG_VIEW_VEND_ID, NULL },                              /* 12 */
  {ID_SET_SERVER_IP, LANG_SET_SERVER_IP, NULL },                            /* 13 */
  {ID_FACTORY_RESET, LANG_FACTORY_RESET, NULL },                            /* 14 */
  /* Administrator */
  {ID_SET_VEND_ID, LANG_VIEW_VEND_ID, NULL },                               /* 15 */
  {ID_INIT_PARAMETER, LANG_INIT_PARAMETER, NULL},                           /* 16 */
  {ID_CHANGE_PASSWORD_1, LANG_CHANGE_PASSWORD_1, NULL},                     /* 17 */
  {ID_CHANGE_PASSWORD_2, LANG_CHANGE_PASSWORD_2, NULL},                     /* 18 */
  /* End menu */
  {ID_ENDING, NULL, NULL }
};

uint8_t g_passwordForm = 0;
static uint8_t g_newUserPassword[6];
static timer_t g_tmrTimeout;
/*********************************************************************************
 * STATIC FUNCTION
 */
static void Keypad_Handler(key_event_t event, uint8_t keyCode);
static void Listbox_Handler(listbox_t* obj, uint32_t selectedIndex);
static void Create_Menu(uint8_t menuType);
static void On_tmrTimeout(timer_t* tmr, void* param);

/*********************************************************************************
 * EXPORTED FUNCTION
 */

void Setting_Show();
void DrawTopGuideLine();
void SetLanguage(language_t language);
void FactoryReset();

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi tong so tien tra lai
 *
 * @param    NONE
 * @retval   NONE
 */
void SettingSetup()
{
    /* Disable NV11*/
    NV11_Disable();
    /* ve huong dan nhan phim tren dong tren cung */
    DrawTopGuideLine();
    /* thoat khoai moi menu cai dat truoc do */
    OutSettingPrice();
    OutSettingSlot();
    OutSettingTime();
    OutSettingEmptyNote();
    OutSettingNoteFill();
    OutSettingOnOff();
    OutSettingViewBillChange();
    OutSettingViewId();
    OutSettingViewSales();
    OutSettingHumidity();
    OutSettingViewError();
    OutSettingViewNoteInOut();
    OutSettingAcceptNote();
    OutSettingRefundNote();
    OutSettingOperatorNumber();
    OutSettingInitParam();
    OutSettingViewFirmwareVer();
    OutSettingTestSlot();
    OutSettingN1Time();
    OutSettingN2Capacity();
    OutSettingN3Capacity();
    OutSettingClearErrorSell();
    /* xoa listbox chua cac menu chinh */
    Lb_Remove(&g_mainListBox);
    /* xoa listbox chua cac menu phu */
    Lb_Remove(&g_subListBox);
    g_interfaceIndex = 0;

    /* them ham xu ly khi co phim nhan */
    Keypad_InstallCallback(KEY_PRESS, Keypad_Handler);
    /* them ham xu ly khi co giu phim */
    Keypad_InstallCallback(KEY_HOLD, Keypad_Handler);
    /* khi g_passwordForm = 0 nhap password de vao menu cai dat */
    g_passwordForm = 0;
    /* hien thi menu nhap mat khau */
    EnterPassword_Show(LANG_ENTER_PASSWORD);
}

/**
 * @brief    Ham xu ly cai dat he thong
 *
 * @param    NONE
 * @retval   NONE
 */
void SettingProcess()
{
    /* thu hien lay cac su kien NV11 */
    NV11_Process();
    /*lay do am cho ban tin gui gia tri do am */
    GetHumidityProcess();
}

/**
 * @brief  Ham soft time tre thoi gian sau 3s nhan cancel de quay ve nhap mat khau
 *
 * @param  timer_t* tmr -> Con tro den dia chi timer
 * @param  void* param -> tham so can truyen vao ham callback
 * @retval NONE
 */
static void On_tmrTimeout(timer_t* tmr, void* param)
{
    g_cancelIsRequested = false;
    Tmr_Stop(&g_tmrTimeout);
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan
 *
 * @param    key_event_t event   -   Su kien khi nhan phim
 * @param    uint8_t keyCode     -   Key code of keypad
 * @retval   NONE
 */
static void Keypad_Handler(key_event_t event, uint8_t keyCode)
{
    /* Neu thao tac la nhan phim */
    if (event == KEY_PRESS)
    {
        /* neu dang hien thi menu cai dat nao thi
           thuc hien xu ly thao tac nhan phim tuong ung menu do */
        if (EnterPassword_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingOnOff_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingSetPrice_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingSetTime_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingEmptyNote_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingNoteFill_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingViewId_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingViewSales_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingSlotNumber_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingViewBillChange_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingSetHumidity_KeyPress(keyCode) == true)
        {
            return;
        }
        if (SettingViewError_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingViewNoteInOut_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingServer_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingAcceptNote_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingRefundNote_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingOperatorNumber_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingVolume_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingViewFirmwareVer_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingInitParam_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingSetN1_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingSetN2_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingSetN3_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingClearErrorSell_KeyPress(keyCode) == true)
        {
            return;
        }

        if (SettingTestSlot_KeyPress(keyCode) == true)
        {
            return;
        }

        /* neu dang hien thi menu phu ma nhan phim cancel
           thi hien thi lai menu chinh */
        if (keyCode == '#' && g_interfaceIndex == 1)     /* CANCEL KEY */
        {
            g_interfaceIndex = 0;
            GLcd_ClearScreen(BLACK);
            DrawTopGuideLine();
            Lb_Show(&g_mainListBox);
        }
        /* neu dang hien thi menu chinh ma nhan phim cancel
           thi hien thi lai nhap mat khau */
        else if (keyCode == '#' && g_interfaceIndex == 0)
        {
            if(g_cancelIsRequested == false)
            {
                g_cancelIsRequested = true;
                /* Khoi tao soft timer, sau 3s thuc hien ham On_tmrTimeout, tham so truyen vao ham la NULL */
                Tmr_Start(&g_tmrTimeout, 3000, On_tmrTimeout, NULL);
            }
            else if(g_cancelIsRequested == true)
            {
                g_cancelIsRequested = false;
                /* Dung bo soft timer */
                Tmr_Stop(&g_tmrTimeout);
                /* khi g_passwordForm = 0 nhap password de vao menu cai dat */
                g_passwordForm = 0;
                /* hien thi menu nhap mat khau */
                EnterPassword_Show(LANG_ENTER_PASSWORD);
            }
        }
        else if (g_interfaceIndex == 0)                /* Neu dang hien thi main menu */
        {
            g_cancelIsRequested = false;
            /* xu ly hien thi menu chinh tren LCD khi co phim nhan */
            Lb_KeyPress(&g_mainListBox, keyCode);
        }
        else if (g_interfaceIndex == 1)                /* Neu dang hien thi menu phu */
        {
            /* xu ly hien thi menu phu tren LCD khi co phim nhan */
            Lb_KeyPress(&g_subListBox, keyCode);
        }
    }
    else if (event == KEY_HOLD)  /* Neu thao tac la giu phim */
    {
        /* neu dang hien thi menu cai dat am luong hoac menu cai dat thoi gian
           thuc hien xu ly thao tac nhan phim tuong ung menu do */
        if (SettingVolume_KeyHold(keyCode) == true)
        {
            return;
        }

        if (SettingSetTime_KeyHold(keyCode) == true)
        {
            return;
        }
    }
}

/**
 * @brief    Ham xu ly du lieu khi dang hien thi menu va co phim nhan la ENTER
 *
 * @param    listbox_t* obj             -   Con tro den listbox hien thi menu
 * @param    uint32_t selectedIndex     -   vi tri text trong menu dang hien thi tren LCD
 *
 * @retval   NONE
 */
static void Listbox_Handler(listbox_t* obj, uint32_t selectedIndex)
{
    menu_item_t* subItem;
    /* neu dang hien thi menu chinh */
    if (obj == &g_mainListBox)
    {
	/* neu hien thi menu thuong */
        if(g_rankAccess != ADMIN_MENU)
	{
	  /* lay id cua dong ma con tro dang hien thi */
	  g_mainMenuId = g_mainMenuItems[selectedIndex].id;
	  /* nhay den ham hien thi tuong ung voi text duoc hien thi tren LCD */
	  if ((g_mainMenuItems[selectedIndex].id == ID_HUMIDITY_ONOFF) ||
	      (g_mainMenuItems[selectedIndex].id == ID_DROP_SENSOR))
	  {
	      SettingOnOff_Show(Lang_GetText(g_mainMenuItems[selectedIndex].textId), g_mainMenuItems[selectedIndex].id);
	  }
	  else if (g_mainMenuItems[selectedIndex].id == ID_SET_TIME)
	  {
	      SettingSetTime_Show(ID_SET_TIME);
	  }

	  else if (g_mainMenuItems[selectedIndex].id == ID_SET_HUMIDITY)
	  {
	      SettingSetHumidity_Show(ID_SET_HUMIDITY);
	  }
	  else if (g_mainMenuId == ID_VIEW_VEND_ID)
	  {
	      SettingViewId_Show(0x00);
	  }
    else if (g_mainMenuId == ID_TEST_SLOT)
    {
        SettingTestSlot_Show();
    }
	  else if (g_mainMenuId == ID_SLOT_NUMBER)
	  {
	      SettingSlotNumber_Show();
	  }
	  else if (g_mainMenuId == ID_ACCEPT_NOTE)
	  {
	      SettingAcceptNote_Show();
	  }
      else if (g_mainMenuId == ID_SET_CAPACITY)
      {
         SettingSetPrice_Show(ID_SUB_CAPACITY);
      }
	  else if (g_mainMenuId == ID_CHANGE_PASSWORD)
	  {
	    if(g_rankAccess == MACHINE_MENU)
	    {
	      /* chinh sua mk cap 1 */
	      g_setPassIndex = 1;
	    }
	    else if(g_rankAccess == TECHNICAL_MENU)
	    {
	      /* chinh sua mk cap 2 */
	      g_setPassIndex = 2;
	    }
	    /* hien thi menu cai dat */
	    g_passwordForm = 1;
	    EnterPassword_Show(LANG_ENTER_NEW_PASSWORD);
	  }
	  else if (g_mainMenuId == ID_SET_SERVER_IP)
	  {
	      SettingServer_Show();
	  }
	  else if (g_mainMenuId == ID_FACTORY_RESET)
	  {
	      g_passwordForm = 3;
	      EnterPassword_Show(LANG_ENTER_PASSWORD);
	  }
	  else if (g_mainMenuId == ID_OPERATOR_NUMBER)
	  {
	      SettingOperatorNumber_Show();
	  }
	  else if (g_mainMenuId == ID_VIEW_FIRMWARE)
	  {
	    SettingViewFirmwareVer_Show();
	  }
      else if (g_mainMenuId == ID_N1_TIME)
	  {
	    SettingSetN1_Show(0);
	  }
      else if (g_mainMenuId == ID_N2_CAPACITY)
	  {
	    SettingSetN2_Show(0);
	  }
      else if (g_mainMenuId == ID_N3_CAPACITY)
	  {
	    SettingSetN3_Show(0);
	  }
      else if (g_mainMenuId == ID_CLEAR_ERROR_SALE)
	  {
	    SettingClearErrorSell_Show();
	  }

	  if (g_mainMenuItems[selectedIndex].subItems)
	  {
	      /* neu trong menu chinh co menu phu de hien thi
		 thi lay danh sach menu phu cua menu chinh de hien thi */
	      Lb_Remove(&g_subListBox);
	      subItem = (menu_item_t*)g_mainMenuItems[selectedIndex].subItems;
	      while (subItem->id != ID_ENDING)
	      {
    		  Lb_AddItem(&g_subListBox, Lang_GetText(subItem->textId));
    		  subItem++;
	      }

	      g_interfaceIndex = 1;
	      Lb_Show(&g_subListBox);
	  }
	}
	/* neu hien thi menu cho admin */
	else if(g_rankAccess == ADMIN_MENU)
	{
	  /* lay id cua dong ma con tro dang hien thi */
	  g_mainMenuId = g_mainMenuItems[selectedIndex + 23].id;  /* 23 is index of ID_SET_VEND_ID */
	  if (g_mainMenuId == ID_SET_VEND_ID)
	  {
	    SettingViewId_Show(0x01);
	  }
	  else if (g_mainMenuId == ID_INIT_PARAMETER)
	  {
	    SettingInitParam_Show();
	  }
	  else if (g_mainMenuId == ID_CHANGE_PASSWORD_1)
	  {
	      /* lua chon hien thi nhap NEW PASSWORD lan thu nhat*/
	      g_passwordForm = 1;
	      g_setPassIndex = 1;
	      EnterPassword_Show(LANG_ENTER_NEW_PASSWORD);
	  }
	  else if (g_mainMenuId == ID_CHANGE_PASSWORD_2)
	  {
	      /* lua chon hien thi nhap NEW PASSWORD lan thu nhat*/
	      g_passwordForm = 1;
	      g_setPassIndex = 2;
	      EnterPassword_Show(LANG_ENTER_NEW_PASSWORD);
	  }
	}
    }
    /* neu dang hien thi menu phu */
    else if (obj == &g_subListBox)
    {
        /* neu manu chinh truoc do la tong doanh so ban hang */
        if (g_mainMenuId == ID_TOTAL_SALES)
        {
            /* neu lua chon hien thi tong doanh thu theo ngay */
            if (selectedIndex == 0)
            {
                SettingViewSales_Show(ID_SALE_IN_DATE);
            }
            /* neu lua chon hien thi tong doanh thu theo thang */
            else if (selectedIndex == 1)
            {
                SettingViewSales_Show(ID_SALE_IN_MONTH);
            }
            /* neu lua chon hien thi tong doanh thu theo nam */
            else if (selectedIndex == 2)
            {
                SettingViewSales_Show(ID_SALE_IN_YEAR);
            }
            /* neu lua chon hien thi tong doanh thu  */
            else if (selectedIndex == 3)
            {
                SettingViewSales_Show(ID_SALE_IN_TOTAL);
            }
        }
        /* neu manu chinh truoc do la cai dat xem loi */
        else if (g_mainMenuId == ID_ERROR_LOG)
        {
            /* neu lua chon hien thi motor loi */
            if (selectedIndex == 0)
            {
                SettingViewError_Show(0);
            }
            /* neu lua chon hien thi loi cua NV11 */
            else if (selectedIndex == 1)
            {
                SettingViewError_Show(1);
            }
            /* neu lua chon hien thi loi cam bien roi */
            else if (selectedIndex == 2)
            {
                SettingViewError_Show(2);
            }
            /* neu lua chon xoa loi dong co*/
            else if (selectedIndex == 3)
            {
                for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
                {
                    Perh_UnlockItem(i);
                }
                /* xoa loi dong co trong EEPROM */
                Perh_SaveItemsToEeprom();
                /* Tao frame thong bao trang thai motor */
                SendMotorResumeFrame();
                /* hien thi menu */
                GLcd_ClearScreen(BLACK);
                int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
                GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                GLcd_Flush();
                Delay(1500);
                /* quay lai hien thi menu */
                Setting_Show();
            }
            /* neu lua chon xoa loi phan cung */
            else if (selectedIndex == 4)
            {
                /* tao frame thong bao trang thai loi phan cung */
                SendRtcResumeFrame();
                SendEepromResumeFrame();
                SendSDCardResumeFrame();
                /* hien thi menu */
                GLcd_ClearScreen(BLACK);
                int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
                GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                GLcd_Flush();
                Delay(1500);
                /* quay lai hien thi menu */
                Setting_Show();
            }
            /* neu lua chon xoa loi cam bien roi */
            else if (selectedIndex == 5)
            {
                Perh_ClearErrorDropSensor();
                /* hien thi menu */
                GLcd_ClearScreen(BLACK);
                int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
                GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                GLcd_Flush();
                Delay(1500);
                /* quay lai hien thi menu */
                Setting_Show();
            }
        }
        else if (g_mainMenuId == ID_NOTE_INOUT)
        {
            /* neu lua chon xem tong so tien nhan va tra */
            if (selectedIndex == 0)
            {
                SettingViewNoteInOut_Show();
            }
            /* neu lua chon xoa so du trong may */
            else if (selectedIndex == 1)
            {
                ClearBalance();
                /* hien thi menu */
                GLcd_ClearScreen(BLACK);
                int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
                GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                GLcd_Flush();
                Delay(1500);
                /* quay lai hien thi menu */
                Setting_Show();
            }
        }
        /* neu menu chinh truoc do la tien tra lai */
        else if (g_mainMenuId == ID_BILL_CHANGE)
        {
            /* neu lua chon cai dat tra lai tien */
            if (selectedIndex == 0)
            {
                SettingOnOff_Show(Lang_GetText(LANG_ONOFF), ID_BILL_CHANGE);
            }
            /* neu lua chon dua tien tra lai vao may */
            else if (selectedIndex == 1)
            {
                if (Perh_RefundIsOn())
                {
                    SettingNoteFill_Show();
                }
            }
            /* neu lua chon lay tien tra lai  */
            else if (selectedIndex == 2)
            {
                if (Perh_RefundIsOn())
                {
                    SettingEmptyNote_Show();
                }
            }
            /* neu lua chon xem tong so tien tra lai trong may */
            else
            {
                if (Perh_RefundIsOn())
                {
                    SettingViewBillChange_Show();
                }
            }
        }
        /* neu menu chinh truoc do la lua chon ngon ngu hien thi */
        else if (g_mainMenuId == ID_LANGUAGE)
        {
            /* VIETNAMESE */
            if (selectedIndex == 0)
            {
                //Wav_Play("ToiVanNho.wav");
                SetLanguage(LANG_VNI);
            }
            /* ENGLISH */
            else if (selectedIndex == 1)
            {
                //Wav_Stop();
                SetLanguage(LANG_ENG);
            }
            Setting_Show();
        }
        /* neu menu chinh truoc do cai dat audio */
        else if (g_mainMenuId == ID_AUDIO_SETTING)
        {
             /* ON/OFF */
            if (selectedIndex == 0)
            {
                SettingOnOff_Show(Lang_GetText(LANG_AUDIO_ONOFF), ID_AUDIO_SETTING);
            }
            /* MAIN VOLUME */
            else if (selectedIndex == 1)
            {
                SettingVolume_Show(Lang_GetText(LANG_MAIN_VOLUME), 0);
            }
            /* CALL VOLUME */
            else if (selectedIndex == 2)
            {
                SettingVolume_Show(Lang_GetText(LANG_CALL_VOLUME), 1);
            }
            /* MICROPHONE GAIN LEVEL */
            else if (selectedIndex == 3)
            {
                SettingVolume_Show(Lang_GetText(LANG_MICROPHONE_LEVEL), 2);
            }
        }
    }
}

static void Create_Menu(uint8_t menuType)
{
  /* init variable */
  uint8_t i = 0;
  uint8_t positionStart = 0;
  uint8_t positionStop = 0;

  Lb_Remove(&g_mainListBox);
  Lb_Remove(&g_subListBox);
  /* calculate position */
  if(menuType == MACHINE_MENU)
  {
    positionStart = 0;
    positionStop = 9;
    g_rankAccess = MACHINE_MENU;
  }
  else if(menuType == TECHNICAL_MENU)
  {
    positionStart = 0;
    positionStop = 15;
    g_rankAccess = TECHNICAL_MENU;
  }
  else if(menuType == ADMIN_MENU)
  {
    positionStart = 15;
    positionStop = 18;
    g_rankAccess = ADMIN_MENU;
  }

  for(i = positionStart; i <= positionStop ; i++)
  {

    /* neu dau doc tien NV9 thi khong cho phep vao menu xem tien tra lai */
    if (g_mainMenuItems[i].id == ID_BILL_CHANGE && NV11_GetUnitType() == NV_TYPE_NV9)
    {
        g_mainMenuItems[i].subItems = NULL;
    }
    /* them thanh phan hien thi vao danh sach menu chinh */
    Lb_AddItem(&g_mainListBox, Lang_GetText(g_mainMenuItems[i].textId));
  }

  /* them hien thi du lieu khi nhan phim enter cua menu chinh */
  Lb_ListenEvents(&g_mainListBox, Listbox_Handler);
  /* them hien thi du lieu khi nhan phim enter cua menu phu */
  Lb_ListenEvents(&g_subListBox, Listbox_Handler);
}

/**
 * @brief    Hien thi text menu chinh hoac menu phu
 *
 * @param    NONE
 * @retval   NONE
 */
void Setting_Show()
{
    /* ve background */
    GLcd_ClearScreen(BLACK);
    if (g_interfaceIndex == 0 || g_interfaceIndex == 1)
    {
        /* ve huong dan nhan phim tren dong tren cung */
        DrawTopGuideLine();
        /* neu dang giao dien menu chinh thi hien thi menu chinh ra LCD */
        if (g_interfaceIndex == 0)
        {
            Lb_Show(&g_mainListBox);
        }
        /* neu dang giao dien menu phu thi hien thi menu phu ra LCD */
        if (g_interfaceIndex == 1)
        {
            Lb_Show(&g_subListBox);
        }
    }
}

/**
 * @brief    Ham ve huong dan nhan phim tren dong tren cung LCD
 *
 * @param    NONE
 * @retval   NONE
 */
void DrawTopGuideLine()
{
    GLcd_DrawBitmap(&g_img_setting_top, 3, 2);
}

/**
 * @brief    Cai dat lai ngon ngu hien thi tren LCD
 *
 * @param    language_t language - lua chon ngon ngu hien thi LCD
 * @retval   NONE
 */
void SetLanguage(language_t language)
{
    uint8_t i = 0;
    /* neu ngon ngu lua chon la ngon ngu hien tai thi return */
    if (language == GetCurrentLanguage())
    {
        return;
    }
    /* luu tham so ngon ngu duoc lua chon vao EEPROM */
    SetCurrentLanguage(language);
    /* xoa listbox hien thi hien tai */
    Lb_Remove(&g_mainListBox);
    Lb_Remove(&g_subListBox);
    /* dua ve hien thi menu chinh */
    g_interfaceIndex = 0;
    /* cap nhat text vao listbox hien thi */
    while (g_mainMenuItems[i].id != ID_ENDING)
    {
        Lb_AddItem(&g_mainListBox, Lang_GetText(g_mainMenuItems[i].textId));
        i++;
    }
}

/**
 * @brief    Ham kiem tra va xu ly du lieu khi nhap password
 * @note     g_passwordForm = 0   - kiem tra PASSWORD khi dang nhap vao may
 *           g_passwordForm = 1   - xu ly du lieu khi nhap PASSWORD moi lan thu nhat
 *           g_passwordForm = 2   - xu ly du lieu khi nhap PASSWORD moi lan thu hai
 *           g_passwordForm = 3   - kiem tra PASSWORD khi tien hanh factory reset
 *           g_passwordForm = 4   - hien thi canh bao lan cuoi khi tien hanh factory reset
 *           g_passwordForm = 5   - factory reset
 *
 * @param    uint8_t* password - con tro toi du lieu password
 * @retval   NONE
 */
void Setting_UpdatePassword(uint8_t* password)
{
    uint8_t i;
    /* neu nhap password de cai cat menu */
    if (g_passwordForm == 0)    /* Enter password */
    {
        /* lay du lieu password lv1 tu EEPROM vao g_newUserPassword */
        Perh_GetShopPassword(g_newUserPassword);
        /* Compare password lv1 */
        if(password[0] == g_newUserPassword[0] && password[1] == g_newUserPassword[1] &&
           password[2] == g_newUserPassword[2] && password[3] == g_newUserPassword[3] &&
           password[4] == g_newUserPassword[4] && password[5] == g_newUserPassword[5])
        {
          gPasswordInTrue = true;
          Create_Menu(MACHINE_MENU);
          Setting_Show();
        }
        else
        {
          /* lay du lieu password lv2 tu EEPROM vao g_newUserPassword */
          Perh_GetTechPassword(g_newUserPassword);
          if(password[0] == g_newUserPassword[0] && password[1] == g_newUserPassword[1] &&
             password[2] == g_newUserPassword[2] && password[3] == g_newUserPassword[3] &&
             password[4] == g_newUserPassword[4] && password[5] == g_newUserPassword[5])
          {
            gPasswordInTrue = true;
            Create_Menu(TECHNICAL_MENU);
            Setting_Show();
          }
          else
          {
            if(password[0] == '8' && password[1] == '8' &&
               password[2] == '9' && password[3] == '5' &&
               password[4] == '9' && password[5] == '5')
            {
              gPasswordInTrue = true;
              Create_Menu(ADMIN_MENU);
              Setting_Show();
            }
            else
            {
              EnterPassword_Show(LANG_ENTER_PASSWORD);
            }
          }
        }
    }
    else if (g_passwordForm == 1)   /* Enter new password */
    {
        /* Save the password that user just typed */
        for (i = 0; i < 6; i++)
        {
            g_newUserPassword[i] = password[i];
        }
        g_passwordForm = 2;
        /* hien thi moi nhap lai mat khau len LCD */
        EnterPassword_Show(LANG_ENTER_PASSWORD_AGAIN);
    }
    else if (g_passwordForm == 2)   /* Enter password again */
    {
        /* Compare the 2 passwords that user typed */
        for (i = 0; i < 6; i++)
        {
            if (g_newUserPassword[i] != password[i])
            {
                /* Hien thi thong bao mat khau khong khop sau do hien thi lai menu */
                GLcd_ClearScreen(BLACK);
                int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_PASSWORD_NOT_MATCH));
                GLcd_DrawStringUni(Lang_GetText(LANG_PASSWORD_NOT_MATCH), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                GLcd_Flush();
                Delay(1500);
                Setting_Show();
                break;
            }
        }
        /* Hien thi thong bao mat khau dung sau do hien thi menu chinh */
        if (i == 6)
	{
	    if(g_setPassIndex == 1)
	    {
	      Perh_SetShopPassword(g_newUserPassword);
	    }
            else
	    {
	      Perh_SetTechPassword(g_newUserPassword);
	    }
            GLcd_ClearScreen(BLACK);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_DONE));
            GLcd_DrawStringUni(Lang_GetText(LANG_DONE), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
            GLcd_Flush();
	    Delay(1500);
            Setting_Show();
        }
    }
    else if (g_passwordForm == 3)   /* Confirm for factory reset */
    {
        /* lay du lieu password tu EEPROM vao g_newUserPassword */
        Perh_SetTechPassword(g_newUserPassword);
        /* Compare password */
        for (i = 0; i < 6; i++)
        {
            if (password[i] != g_newUserPassword[i])
            {
                /* Hien thi thong bao mat khau khong khop sau do hien thi lai menu */
                GLcd_ClearScreen(BLACK);
                int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_PASSWORD_NOT_MATCH));
                GLcd_DrawStringUni(Lang_GetText(LANG_PASSWORD_NOT_MATCH), (GLCD_WIDTH - sizeInPixel) / 2, 30, WHITE);
                GLcd_Flush();
                Delay(1500);
                OutSettingPassWord();
                Setting_Show();
                break;
            }
        }
        if (i == 6)
        {
          g_passwordForm = 4;
          /* password lan 2 dung hien thi canh bao lan cuoi */
          GLcd_ClearScreen(BLACK);
          int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_CONFIRM_RESET));
          GLcd_DrawStringUni(Lang_GetText(LANG_CONFIRM_RESET), (GLCD_WIDTH - sizeInPixel) / 2, 25, WHITE);

          sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_YES_NO));
          GLcd_DrawStringUni(Lang_GetText(LANG_YES_NO), (GLCD_WIDTH - sizeInPixel) / 2, 50, WHITE);

          GLcd_Flush();
        }
    }
    else if(g_passwordForm == 4)
    {
      /* trang thai cho xac nhan factory reset */
    }
    else if(g_passwordForm == 5) /* factory reset */
    {
      /* tien hanh factory reset */
      FactoryReset();
      Dbg_Println("Setting >> Factory Reset");
      __NVIC_SystemReset();
    }
}

/**
 * @brief    Ham xu ly du lieu, dua cac tham so cai dat ve gia tri mac dinh
 *
 * @param    NONE
 * @retval   NONE
 */
void FactoryReset()
{
    int i = 0;
    /* Reset tham so cot la 10 va hang la 6, reset tham so Item_ID */
    Perh_SetSlotColumn(10);
    Perh_SetSlotRow(6);
    Perh_UpdateItemId();
    Perh_ProcessAllWhile();
    /* Reset so hang trong moi ngan va gia tien cua moi ngan */
    for (i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        Perh_SetItemNumber(i, 0);
        Perh_SetItemPrice(i, 0);
        Perh_UnlockItem(i);
    }
    /* Bat chuong trinh header va luu trang thai vao EEPROM  */
    Perh_HeaterOn();
    /* Bat cam bien roi va luu trang thai vao EEPROM  */
    Perh_DropSensorOn();

    /* Dua gia tri cai dat do am ve 10 */
    Perh_SetHumidity(80);

    /* Dua tong so tien nhan va tra ve 0 */
    Perh_ResetNoteInOut();

    /* Xoa so du va dat lai so tien nhan bang 0 */
    Perh_SaveBalance(0);
    Perh_SaveReceivedNotes(0);

    /* Dua tham so Server ve gia tri mac dinh */
    Gsm_SetServerInfo(203, 171, 20, 62, 9201u);

    /* Reset password lv 1 */
    uint8_t initPassword[6];
    for (uint8_t i = 0; i < 6; i++)
    {
        initPassword[i] = '0';
    }
    Perh_SetShopPassword(initPassword);

    /* luu toan bo thong tin hang hoa */
    Perh_SaveItemsToEeprom();

    /* Delete the frame buffer for GPRS */
    uint32_t tmp = 0, tmp1 = 0, failCount = 0;
    EE_Write(GPRS_FRAME_EEP_ADDRESS, (uint8_t*)(&tmp), 4);
    /* Ghi dia chi PUT du lieu vao EEPROM */
    tmp = GPRS_FRAME_EEP_ADDRESS + 200;
    EE_Write(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, (uint8_t*)(&tmp), 4);
    Perh_ProcessAllWhile();
    /* Verify the written data */
    while (1)
    {
        tmp1 = 0;
        EE_Read(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, &tmp1, 4);
        if (tmp1 != GPRS_FRAME_EEP_ADDRESS + 200)
        {
            failCount++;
            if (failCount > 10)
            {
                return;
            }
            EE_Write(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, (uint8_t*)(&tmp), 4);
        }
        else
        {
            break;
        }
    }
    /* Ghi dia chi GET du lieu vao EEPROM */
    tmp1 = 0;
    EE_Write(GPRS_FRAME_GET_PTR_EEP_ADDRESS, (uint8_t*)(&tmp), 4);
    /* Verify the written data */
    while (1)
    {
        tmp1 = 0;
        EE_Read(GPRS_FRAME_GET_PTR_EEP_ADDRESS, &tmp1, 4);
        if (tmp1 != GPRS_FRAME_EEP_ADDRESS + 200)
        {
            EE_Write(GPRS_FRAME_GET_PTR_EEP_ADDRESS, (uint8_t*)(&tmp), 4);
        }
        else
        {
            break;
        }
    }
    /* Update g_eepromBufferPutPtr and g_eepromBufferGetPtr trong gsm */
    RestoreEepBufferPointer();
    /* Cai dat am luong cuoc goi o muc cao nhat */
    Perh_SetCallVolume(100);
    /* Cai dat am luong media o muc cao nhat */
    Perh_SetMediaVolume(100);
    /* Cai dat muc thu mic o muc cao nhat */
    Perh_SetMicrophoneLevel(15);
    /* Dat muc tien tra lai ve 5000 */
    Perh_SaveRefundNote(0xFF, false);   /* Route all channel to cash box */
    Perh_SaveRefundNote(2, true);       /* Route 5000VND to floating box */

    /* xoa loi cam bien roi */
    Perh_ClearErrorDropSensor();
}


/**
 * @brief    Thoat khoi menu cai dat
 *
 * @param    NONE
 * @retval   NONE
 */
void Setting_Exit()
{
    OutSettingTime();
}
