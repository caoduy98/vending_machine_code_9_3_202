
#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include "platform.h"
#include "eeprom_memory_map.h"
/* The EEPROM address */
#define STORED_BALANCE_EEP_ADDRESS          ADDR_EEPROM_PAGE_4     /* The balance for the last session */
#define STORED_RECEIVED_NOTES_EEP_ADDRESS   ADDR_EEPROM_PAGE_5     /* The total of notes in for the last session */
#define MONEY_IN_EEP_ADDRESS                ADDR_EEPROM_PAGE_6     /* The gross total of money moved in */
#define MONEY_OUT_EEP_ADDRESS               ADDR_EEPROM_PAGE_7     /* The gross total of money moved out (changed money) */
#define GPRS_FRAME_PUT_PTR_EEP_ADDRESS      ADDR_EEPROM_PAGE_8
#define GPRS_FRAME_GET_PTR_EEP_ADDRESS      ADDR_EEPROM_PAGE_9
/* page 10 parameter */
#define INIT_EEP_ADDRESS                    ADDR_EEPROM_PAGE_10 + 0
#define LED_EEP_ADDRESS                     ADDR_EEPROM_PAGE_10 + 1       /* LED - 1 byte */
#define DROP_SENSOR_EEP_ADDRESS             ADDR_EEPROM_PAGE_10 + 2       /* Drop Sensor - 1 byte */
#define SLOT_COLUMNS_EEP_ADDRESS            ADDR_EEPROM_PAGE_10 + 3       /* Column number - 1 byte */
#define SLOT_ROWS_EEP_ADDRESS               ADDR_EEPROM_PAGE_10 + 4       /* Row number - 1 byte */
#define HUMIDITY_EEP_ADDRESS                ADDR_EEPROM_PAGE_10 + 5       /* Humidity set point - 1 byte */
#define LANGUAGE_EEP_ADDRESS                ADDR_EEPROM_PAGE_10 + 6       /* Language - 1 byte */
#define VEND_ID_EEP_ADDRESS                 ADDR_EEPROM_PAGE_10 + 7       /* Machine ID - 6 bytes */
#define SHOP_PASSWORD_EEP_ADDRESS           ADDR_EEPROM_PAGE_10 + 13      /* Password for store owner - 6 bytes */
#define TECH_PASSWORD_EEP_ADDRESS           ADDR_EEPROM_PAGE_10 + 19      /* Password for technician - 6 bytes */
#define FACTORY_PASSWORD_EEP_ADDRESS        ADDR_EEPROM_PAGE_10 + 25      /* Password for factory - 6 bytes */
#define MOBIILE_EEP_ADDRESS                 ADDR_EEPROM_PAGE_10 + 31      /* Store owner mobile number - 15 bytes */
#define SERVER_IP_EEP_ADDRESS               ADDR_EEPROM_PAGE_10 + 46      /* Server IP address - 4 bytes */
#define ACCEPT_NOTE_MAX_EEP_ADDRESS_LOW     ADDR_EEPROM_PAGE_10 + 50      /* Maximum value of acceptable note low - 1 bytes */
#define AUDIO_EEP_ADDRESS                   ADDR_EEPROM_PAGE_10 + 51      /* Audio on/off - 1 byte */
#define OP_NUM_EEP_ADDRESS                  ADDR_EEPROM_PAGE_10 + 52      /* Operator number - 16 byte */
#define CALL_VOLUME_EEP_ADDRESS             ADDR_EEPROM_PAGE_10 + 68      /* Volume of the audio during a call - 1 byte */
#define MEDIA_VOLUME_EEP_ADDRESS            ADDR_EEPROM_PAGE_10 + 69      /* Volume of the media - 1 byte */
#define MICROPHONE_LEVEL_EEP_ADDRESS        ADDR_EEPROM_PAGE_10 + 70      /* Level of the microphone during a call - 1 byte */
#define REFUND_ONOFF_EEP_ADDRESS            ADDR_EEPROM_PAGE_10 + 71      /* Refunds enable/disable - 1 byte */
#define SERVER_PORT_EEP_ADDRESS             ADDR_EEPROM_PAGE_10 + 72      /* Server Port - 4 bytes */
#define REFUND_NOTE_EEP_ADDRESS             ADDR_EEPROM_PAGE_10 + 76      /* Refund note - 1 bytes */
#define STATE_ERROR_PAYOUT_ADDRESS          ADDR_EEPROM_PAGE_10 + 78      /* State error payout - 1 bytes */
#define STATE_ERROR_DROP_SENDOR             ADDR_EEPROM_PAGE_10 + 79      /* State error dropsensor - 1 byte */
#define ACCEPT_NOTE_MAX_EEP_ADDRESS_HIGH    ADDR_EEPROM_PAGE_10 + 80      /* Maximum value of acceptable note high - 1 byte (500K)*/
#define N1_TIME_ADDRESS                     ADDR_EEPROM_PAGE_10 + 81      /* 4 byte */
#define N2_CAPACITY_ADDRESS                 ADDR_EEPROM_PAGE_10 + 85      /* 4 byte */
#define N3_CAPACITY_ADDRESS                 ADDR_EEPROM_PAGE_10 + 89      /* 4 byte */
/* page 11 update information */
#define UPDATE_AUTO_FLAG_ADDRESS            ADDR_EEPROM_PAGE_11
#define LENGTH_FILE_UPDATE_NAME_ADDRESS     UPDATE_AUTO_FLAG_ADDRESS          + 1
#define FILE_UPDATE_NAME_ADDRESS            LENGTH_FILE_UPDATE_NAME_ADDRESS   + 1
#define VERSION_CURRENT                     FILE_UPDATE_NAME_ADDRESS          + 16
#define FILE_UPDATE_CRC_ERROR_ADDRESS       VERSION_CURRENT                   + 16
/* page 20 - page 30 item information */
#define ITEM_INFO_EEP_ADDRESS               ADDR_EEPROM_PAGE_20           /* The start address to store the information for an item slot (1 motor) */
/* page 31 - page 112 gsm communication frame */
#define GPRS_FRAME_EEP_ADDRESS              ADDR_EEPROM_PAGE_31
/* page 120 - 164 alarm frame */

/* page 165 - 510 not using */
/* page 511 file debug */
#define COUNT_FILE_DEBUG                    ADDR_EEPROM_PAGE_511          /* Count file debug in day - 4 byte*/

typedef struct
{
    uint8_t id;
    uint32_t price;
    uint8_t available;
    bool isLocked;
} item_t;

#define BEEP_PIN            PB8

void Perh_Init();
void Perh_HeaterOn();
void Perh_HeaterOff();
bool Perh_HeaterIsOn();
void Perh_DropSensorOn();
void Perh_DropSensorOff();
bool Perh_DropSensorIsOn();
void Perh_AudioOn();
void Perh_AudioOff();
bool Perh_AudioIsOn();
void Perh_RefundOn();
void Perh_RefundOff();
bool Perh_RefundIsOn();
bool Perh_ItemIsDetected();
void Perh_ClearDropSensorFlag();
void Perh_SetVendId(const uint8_t* id);
void Perh_GetVendId(uint8_t* id);
void Perh_SetShopPassword(uint8_t* password);
void Perh_GetShopPassword(uint8_t* password);
void Perh_SetTechPassword(uint8_t* password);
void Perh_GetTechPassword(uint8_t* password);

void Perh_SetSlotColumn(uint8_t columns);
void Perh_SetSlotRow(uint8_t rows);
uint8_t Perh_GetSlotColumn();
uint8_t Perh_GetSlotRow();
void Perh_SetHumidity(uint8_t humidity);
uint8_t Perh_GetHumidity();
uint8_t Perh_GetAcceptableNoteLow();
uint8_t Perh_GetAcceptableNoteHight();
void Perh_SaveAcceptableNoteLow(uint8_t note);
void Perh_SaveAcceptableNoteHigh(uint8_t note);
void Perh_SaveBalance(uint32_t balance);
void Perh_SaveReceivedNotes(uint32_t receivedNotes);
void Perh_RestoreBalance(uint32_t* balance);
void Perh_RestoreReceivedNotes(uint32_t* receivedNotes);
void Perh_AddNoteIn(uint32_t amount);
void Perh_AddNoteOut(uint32_t amount);
void Perh_ResetNoteInOut();
uint32_t Perh_GetTotalNoteIn();
uint32_t Perh_GetTotalNoteOut();
void Perh_GetServerInfo(uint8_t* ip1 ,uint8_t* ip2, uint8_t* ip3, uint8_t* ip4, uint32_t* port);
void Perh_SaveServerInfo(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4, uint32_t port);
void Perh_SaveRefundNote(uint8_t channel, bool routToPayout);
bool Perh_GetRefundNote(uint8_t channel);

bool Perh_GetOperatorNumber(char* number);
bool Perh_SetOperatorNumber(const char* number);
uint8_t Perh_GetCallVolume();   /* Return percents of volume */
void Perh_SetCallVolume(uint8_t value);
uint8_t Perh_GetMediaVolume();
void Perh_SetMediaVolume(uint8_t value);
uint8_t Perh_GetMicrophoneLevel();      /* Return from 0 to 15 */
void Perh_SetMicrophoneLevel(uint8_t value);

void Perh_UpdateItemId();
void Perh_SetItemPrice(int id, uint32_t price);
void Perh_SetItemNumber(int id, uint8_t number);
uint32_t Perh_GetItemPrice(int id);
uint8_t Perh_GetItemNumber(int id);
void Perh_LockItem(int id);
void Perh_UnlockItem(int id);
bool Perh_ItemIsLocked(int id);
item_t* Perh_GetItem(int id);
bool Perh_ItemIsExist(int id);
bool Perh_ItemIsAvailable(int id);
void Perh_SaveItemsToEeprom();

void Perh_GetFirmwareVersion(uint8_t* fir_ver);
void Perh_InitFirmwareVersionName(uint8_t* fir_ver, uint8_t lenName);

void Perh_SetStateErrorPayout(bool state);
bool Perh_GetStateErrorPayout();

void Perh_CheckDropSensorProcess(void);
void Perh_ProcessAllWhile(void);
void Perh_SetErrorDropSensor(void);
void Perh_ClearErrorDropSensor(void);
uint8_t Perh_GetErrorDropSensor(void);

void Perh_SetCountFileDebug(uint32_t count_file);
uint32_t Perh_GetCountFileDebug(void);


uint32_t Perh_GetN1Time();
void Perh_SetN1Time(uint32_t u32N1Time);
uint32_t Perh_GetN2Capacity();
void Perh_SetN2Capacity(uint32_t u32N2);
uint32_t Perh_GetN3Capacity();
void Perh_SetN3Capacity(uint32_t u32N3);
#endif  /* PERIPHERAL_H */
