#ifndef SETTING_ID_H
#define SETTING_ID_H

#include "language.h"

extern const uint8_t g_img_tien_tra_lai[];

extern const uint8_t g_img_setting_top[];
extern const uint8_t g_img_total_sales[];
extern const uint8_t g_img_key_slot[];
extern const uint8_t g_img_price_capacity[];
extern const uint8_t g_img_error_log[];
extern const uint8_t g_img_test_slot[];
extern const uint8_t g_img_note_inout[];
extern const uint8_t g_img_led_onoff[];
extern const uint8_t g_img_drop_sensor[];
extern const uint8_t g_img_set_temperature[];
extern const uint8_t g_img_set_time[];
extern const uint8_t g_img_note_fill[];
extern const uint8_t g_img_note_recycler[];
extern const uint8_t g_img_change_password[];
extern const uint8_t g_img_view_id[];

extern const uint8_t g_img_off[];
extern const uint8_t g_img_on[];
extern const uint8_t g_img_theo_ngay[];
extern const uint8_t g_img_theo_thang[];
extern const uint8_t g_img_theo_nam[];
extern const uint8_t g_img_tong_doanh_thu[];
extern const uint8_t g_img_unit_price[];
extern const uint8_t g_img_capacity[];

void DrawTopGuideLine();
void Setting_UpdatePassword(uint8_t* password);

void EnterPassword_Show(language_id_t languageId);
bool EnterPassword_KeyPress(uint8_t keyCode);

void SettingOnOff_Show(const uint16_t* title, uint32_t id);
bool SettingOnOff_KeyPress(uint8_t keyCode);

void SettingSetPrice_Show(uint32_t id);
bool SettingSetPrice_KeyPress(uint8_t keyCode);

void SettingSetTime_Show(uint32_t id);
bool SettingSetTime_KeyPress(uint8_t keyCode);
bool SettingSetTime_KeyHold(uint8_t keyCode);

void SettingEmptyNote_Show();
bool SettingEmptyNote_KeyPress(uint8_t keyCode);

void SettingNoteFill_Show();
bool SettingNoteFill_KeyPress(uint8_t keyCode);

void SettingViewBillChange_Show();
bool SettingViewBillChange_KeyPress(uint8_t keyCode);

void SettingViewSales_Show(uint32_t id);
bool SettingViewSales_KeyPress(uint8_t keyCode);

void SettingViewId_Show(uint8_t menuState);
bool SettingViewId_KeyPress(uint8_t keyCode);

void SettingSlotNumber_Show();
bool SettingSlotNumber_KeyPress(uint8_t keyCode);

void SettingSetHumidity_Show(uint32_t id);
bool SettingSetHumidity_KeyPress(uint8_t keyCode);

void SettingViewError_Show(uint32_t id);
bool SettingViewError_KeyPress(uint8_t keyCode);

void SettingViewNoteInOut_Show();
bool SettingViewNoteInOut_KeyPress(uint8_t keyCode);

void SettingAcceptNote_Show();
bool SettingAcceptNote_KeyPress(uint8_t keyCode);

void SettingRefundNote_Show(uint8_t channelNumber);
bool SettingRefundNote_KeyPress(uint8_t keyCode);

void SettingOperatorNumber_Show();
bool SettingOperatorNumber_KeyPress(uint8_t keyCode);

void SettingVolume_Show(const uint16_t* title, uint32_t id);
bool SettingVolume_KeyPress(uint8_t keyCode);
bool SettingVolume_KeyHold(uint8_t keyCode);

void SettingServer_Show();
bool SettingServer_KeyPress(uint8_t keyCode);

void SettingViewFirmwareVer_Show();
bool SettingViewFirmwareVer_KeyPress(uint8_t keyCode);

void SettingInitParam_Show();
bool SettingInitParam_KeyPress(uint8_t keyCode);

void SettingTestSlot_Show();
bool SettingTestSlot_KeyPress(uint8_t keyCode);

void SettingSetN1_Show(uint32_t id);
bool SettingSetN1_KeyPress(uint8_t keyCode);

void SettingSetN2_Show(uint32_t id);
bool SettingSetN2_KeyPress(uint8_t keyCode);

void SettingSetN3_Show(uint32_t id);
bool SettingSetN3_KeyPress(uint8_t keyCode);

void SettingClearErrorSell_Show();
bool SettingClearErrorSell_KeyPress(uint8_t keyCode);
/* IDs for main menu items */
enum main_menu_id
{
  /* Machine Operator  + Technical staff */
  ID_TOTAL_SALES,                /* 0 */
  ID_SET_CAPACITY,                  /* 1 */
  ID_ERROR_LOG,                  /* 2 */
  ID_NOTE_INOUT,                 /* 3 */
  ID_HUMIDITY_ONOFF,             /* 4 */
  ID_SET_HUMIDITY,               /* 5 */
  ID_BILL_CHANGE,                /* 6 */
  ID_ACCEPT_NOTE,                /* 7 */
  ID_N1_TIME,                    /* 8 */
  ID_N2_CAPACITY,               /* 9 */
  ID_N3_CAPACITY,               /* 10 */
  ID_CLEAR_ERROR_SALE,          /* 11 */
  ID_LANGUAGE,                   /* 12 */
  ID_CHANGE_PASSWORD,            /* 13 */
  ID_VIEW_FIRMWARE,              /* 14 */
  /* Technical staff */
  ID_DROP_SENSOR,
  ID_TEST_SLOT,                  /* 15 */
  ID_AUDIO_SETTING,              /* 16 */
  ID_SLOT_NUMBER,                /* 17 */
  ID_SET_TIME,                   /* 18 */
  ID_OPERATOR_NUMBER,            /* 19 */
  ID_VIEW_VEND_ID,               /* 20 */
  ID_SET_SERVER_IP,              /* 21 */
  ID_FACTORY_RESET,              /* 22 */
  /* Administrator */
  ID_SET_VEND_ID,                /* 23 */
  ID_INIT_PARAMETER,             /* 24 */
  ID_CHANGE_PASSWORD_1,          /* 25 */
  ID_CHANGE_PASSWORD_2,          /* 26 */
};

/* Sub-IDs for "SET PRICE" */
#define ID_SUB_PRICE        0x00000100U
#define ID_SUB_CAPACITY     0x00000200U

/* Sub-IDs for "Doanh Thu" */
#define ID_SALE_IN_DATE     0x00000300U
#define ID_SALE_IN_MONTH    0x00000400U
#define ID_SALE_IN_YEAR     0x00000500U
#define ID_SALE_IN_TOTAL    0x00000600U

/* ENDING ID */
#define ID_ENDING           0xFFFFFFFFU

#endif  /* SETTING_ID_H */
