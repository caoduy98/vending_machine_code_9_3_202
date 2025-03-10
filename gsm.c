
#include "gsm.h"
#include "string.h"
#include "fsl_lpuart.h"
#include "nv11.h"
#include "crc_calculator.h"
#include "nv11.h"
#include "peripheral.h"
#include "eeprom.h"
#include "ff.h"
#include "glcd.h"
#include "language.h"
#include "ftp_define.h"
#include "watchdog.h"

/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_GSM
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/* The pin t is used to turn on/off the power for GSM module */
#define GSM_PW_PIN      PC5     /* Chan dieu khien nguon cap SIM */
#define GSM_STS_PIN     PE1     /* Chan Status */
#define GSM_RST_PIN     PE0     /* Chan Reset SIM */

#define GSM_RX_PIN      PD17    /* Chan RX */
#define GSM_TX_PIN      PE12    /* Chan TX */

static serial_t g_serial;

static char g_serverIP[30] = "203.171.20.62";         /* Dia chi IP Server */
static char g_serverPort[10] = "9201";                  /* Dia chi Port Server */

FIL g_logFile;
bool g_eepromIsError = false;
bool g_rtcIsError = false;


#define GPRS_FRAME_ON_RAM_SIZE       10
#define GPRS_EEP_BUFFER_START       GPRS_FRAME_EEP_ADDRESS + 200
#define GPRS_EEP_BUFFER_END         (GPRS_EEP_BUFFER_START + 10000u)

typedef struct
{
    uint8_t dataToSend[170];
    uint32_t length;
} gprs_command_t;
/* */

enum{
  RESPONSE_SUCCESS = 0x01,
  RESPONSE_ERROR = 0x02
};

typedef struct
{
  uint8_t success_flag;
  uint8_t update_flag;
  uint8_t version[3];
  struct
  {
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hours;
    uint8_t minute;
    uint8_t seconds;
  }time;
}response_t;

response_t response;
static void Gsm_check_response(response_t* response);
/* */
static uint8_t g_FrameOpenDoor[] = "###|TPA0123456|CMD=|c|000000|00&&&";    // 0x63

static uint8_t g_FrameCloseDoor[] = "###|TPA0123456|CMD=|d|111000000000000000000000000000000000000000000000000000000000000000000000111111111111111111111111111111111111111111111111111111111111000000|00&&&"; // 0x64

static uint8_t g_FrameCycle[] = "###|TPA0123456|CMD=|b|101111111111000000000000000000000000000000000000000000000000000000000000000000000|00&&&"; // 0x62
static uint8_t g_FrameReleasedItem[] = "###|TPA0123456|CMD=|e|11000000|00&&&";  // 0x65
static uint8_t g_FramePaidoutChange[] = "###|TPA0123456|CMD=|i|1000000|00&&&";  // 0x69
static uint8_t g_FrameSellingError[] = "###|TPA0123456|CMD=|f|11000000|00&&&";  // 0x66
static uint8_t g_FrameDeviceError[] = "###|TPA0123456|CMD=|g|11|00&&&";         // 0x67
static uint8_t g_FrameTimeRequest[] = "###|TPA0123456|CMD=|p|1|00&&&";         // 0x70


static gprs_command_t g_frameBufferOnRam[GPRS_FRAME_ON_RAM_SIZE];
static uint8_t g_frameBufferOnRamPutPtr = 0;
static uint8_t g_frameBufferOnRamGetPtr = 0;
static uint32_t g_eepromBufferPutPtr = 0;
static uint32_t g_eepromBufferGetPtr = 0;
static bool g_isFrameFromRam = false;

/* The structure used to stored the frame being sent */
static gprs_command_t g_gprsCurrentFrame;

static uint8_t g_state = 0;         /* State machine for process GPRS */
static uint8_t g_callState = 0;     /* State machine forW making a call */
static uint32_t g_curTick = 0;
static uint32_t g_periodSendingTick = 0;
static uint8_t g_failureCount = 0;

static uint8_t g_uartBuffer[300];
static bool g_isCalling = false;    /* The variable to indicate that the call is taking place */
/* Request to make a call or hang up a call
   0: No request
   1: make a call
   2: hang up a call */
static uint8_t g_callRequest = 0;

uint8_t g_gsmHumidityValueMsg = 0;
float g_gsmHumidityValueCheckErr;
static char FrameToLog[600];

static void SendSsHumidityError();
static void SendSsHumidityResume();

static void Gsm_SendDataPeriod();
static void SendNV11ErrorFrame();
static void SendTimeRequestFrame();
static void SendRtcErrorFrame();
static void SendEepromErrorFrame();

static void WriteVerifyDataFlag(uint32_t address, uint8_t data);
static void ReadVerifyDataFlag(uint32_t address, uint8_t* data);
static void WriteVerifyData(uint32_t address, uint8_t* data, int length);
static void ReadVerifyData(uint32_t address, uint8_t* data, int length);

void SendEepromResumeFrame();
void SendRtcResumeFrame();
void Gsm_CallProcess();

extern FRESULT OpenForAppend(FIL* file, const char* path);

/* static for update firware */
status_update_t updateFirmware = NONE_UPDATE;
static uint8_t g_savestate = 0;
static uint8_t g_updateFileName[32] = {0};
static uint8_t g_lenUpdateFileName = 0;
extern uint8_t g_FirmwareVersion[3];
/* */
static uint8_t g_vmNameFile[32] = {0};
static uint8_t g_updateLastFileName[32] = {0};
static uint8_t g_lenUpdateLastFileName = 0;
/* */
static uint8_t g_ftpErrorCount = 0;
static char buf_str[64];
static uint8_t ftp_temp = 0;
static uint8_t ftp_check_tick = 0;
static uint8_t ftp_count_down = 0;

/* static variable for sign quality */
uint8_t g_signalQuantity = 0;
bool g_detectSimCard = false;
extern uint8_t g_is_half_stock;
static char strGsm[64];
/**
 * @brief  Ham ngat UART2 doc du lieu truyen tu module SIM
 *
 * @param  NONE
 * @retval NONE
 */
void LPUART2_IRQHandler()
{
    uint8_t data;
    /* Kiem tra trang thai co bao ngat nhan UART2 */
    uint32_t status = LPUART_GetStatusFlags(LPUART2);
    /* Co bao bo dem du lieu day */
    if ((kLPUART_RxDataRegFullFlag) & status)
    {
        data = LPUART_ReadByte(LPUART2);                /* Nhan du lieu tu bo dem UART2 */
        Serial_RxHandler(&g_serial, data);              /* Truyen vao bo dem RxBuffer cua nguoi dung */
    }
    /* Bao bo dem bi tran, xay ra khi du lieu moi vao bo dem, du lieu cu chua duoc doc*/
    else if ((kLPUART_RxOverrunFlag) & status)
    {
        LPUART_ClearStatusFlags(LPUART2, kLPUART_RxOverrunFlag);        /* Xoa co bao loi */
        Dbg_Println("Gsm >> LPUART2, kLPUART_RxOverrunFlag");           /* Hien thi loi len man hinh debug */
    }
    /* Lay 3 mau cua mot 1 bit nhan duoc, neu 3 mau khac nhau, co bao nhieu duoc bat */
    else if ((kLPUART_NoiseErrorFlag) & status)
    {
        LPUART_ClearStatusFlags(LPUART2, kLPUART_NoiseErrorFlag);
        Dbg_Println("Gsm >> LPUART2, kLPUART_NoiseErrorFlag");
    }
    /* Co bao loi Frame truyen, co bao loi khi phat hien logic 0 trong bit "Stop"*/
    else if ((kLPUART_FramingErrorFlag) & status)
    {
        LPUART_ClearStatusFlags(LPUART2, kLPUART_FramingErrorFlag);
        Dbg_Println("Gsm >> LPUART2, kLPUART_FramingErrorFlag");
    }
    /* Neu de che do kiem tra chan le (Parity), co bao khi phat hien chan le */
    else if ((kLPUART_ParityErrorFlag) & status)
    {
        LPUART_ClearStatusFlags(LPUART2, kLPUART_ParityErrorFlag);
        Dbg_Println("Gsm >> LPUART2, kLPUART_ParityErrorFlag");
    }
    /* Voi cac loi khac, in ra man hinh debug -> khong nhan dang duoc */
    else
    {
        Dbg_Println("Gsm >> LPUART2, Unknown");
    }
}
/**
 * @brief   Ham luu tru vi tri ghi du lieu trong EEPROM
 *
 * @param   NONE
 * @retval  NONE
 */
static void SaveEepBufferPutPointer()
{
    uint32_t tmp, failCount = 0;
    /* Lay dia chi cua con tro luu vi tri ghi du lieu */
    uint8_t* p = (uint8_t*)(&g_eepromBufferPutPtr);
    /* Ghi du lieu vao EEPROM, do dai 4 byte */
    EE_Write(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, p, 4);
    /* Kiem tra lai qua trinh ghi */
    while (1)
    {
        tmp = 0;
        /* Doc lai du lieu vua ghi vao tmp */
        EE_Read(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, &tmp, 4);
        /* Neu du lieu can ghi va du lieu doc ve khac nhau */
        if (tmp != g_eepromBufferPutPtr)
        {
            failCount++;
            /* Neu so lan ghi loi va ghi lai qua 50 lan -> thoat */
            if (failCount > 50)
            {
                break;
            }
            /* Thuc hien ghi lai */
            EE_Write(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, p, 4);
        }
        /* Neu du lieu ghi ok -> thoat */
        else
        {
            break;
        }
    }
}
/**
 * @brief   Ham luu tru vi tri doc du lieu trong EEPROM
 *
 * @param   NONE
 * @retval  NONE
 */
static void SaveEepBufferGetPointer()
{
    uint32_t tmp, failCount = 0;
    /* Lay dia chi cua con tro luu vi tri du lieu */
    uint8_t* p = (uint8_t*)(&g_eepromBufferGetPtr);
    /* Ghi du lieu vao EEPROM, do dai 4 byte */
    EE_Write(GPRS_FRAME_GET_PTR_EEP_ADDRESS, p, 4);
    /* Kiem tra lai qua trinh ghi */
    while (1)
    {
        tmp = 0;
        /* Doc lai du lieu vua ghi vao tmp */
        EE_Read(GPRS_FRAME_GET_PTR_EEP_ADDRESS, &tmp, 4);
        /* Neu du lieu can ghi va du lieu doc ve khac nhau */
        if (tmp != g_eepromBufferGetPtr)
        {
            failCount++;
            /* Neu so lan ghi loi va ghi lai qua 50 lan -> thoat */
            if (failCount > 50)
            {
                break;
            }
            /* Thuc hien ghi lai */
            EE_Write(GPRS_FRAME_GET_PTR_EEP_ADDRESS, p, 4);
        }
        /* Neu du lieu ghi ok -> thoat */
        else
        {
            break;
        }
    }
}

/**
 * @brief   Ham check sim card IMEI detect sim
 *
 * @param   NONE
 * @retval  NONE
 */
static void DetectSimCard()
{
    Serial_ClearRxBuffer(&g_serial);
    Serial_Write(&g_serial, "AT+CIMI\n\r", 10);       /* Gui lenh check sim card imei */
    Delay(1000);
    if(Serial_FindString(&g_serial, "OK") != NULL)
    {
      g_detectSimCard = true;
      Dbg_Println("Gsm >> Detect Sim Card");
    }
    else if(Serial_FindString(&g_serial, "ERROR") != NULL)
    {
      g_detectSimCard = false;
      Dbg_Println("Gsm >> No Detect Sim Card");
    }
}

/**
 * @brief   Ham thiet lap vi tri du lieu ghi va doc trong EEPROM
 *
 * @param   NONE
 * @retval  NONE
 */
void RestoreEepBufferPointer()
{
    uint8_t failCount = 0;
    uint32_t tmp[10];
    uint8_t i = 0;
    /* Thiet lap lai vi tri du lieu ghi trong EEPROM */
    while (1)
    {
        /* Xoa toan bo tmp */
        for (i = 0; i < 10; i++)
        {
            tmp[i] = 0;
        }
        /* Tien hanh doc 10 lan vi tri du lieu ghi trong EEPROM */
        for (i = 0; i < 10; i++)
        {
            EE_Read(GPRS_FRAME_PUT_PTR_EEP_ADDRESS, &tmp[i], 4);
            Delay(50);
            if (i > 0)
            {
                /* Kiem tra neu du lieu vua doc duoc khac lan doc truoc */
                if (tmp[i] != tmp[i - 1])
                {
                    failCount++;        /* Dem so lan khac */
                    if (failCount > 10)
                    {
                        break;
                    }
                    i = 0;
                }
            }
        }
        /* Neu du lieu qua moi lan doc khac nhau qua 10 lan*/
        if (failCount > 10)
        {
            /* Thiet lap toan bo vi tri du lieu ghi va doc ve vi tri ban dau */
            g_eepromBufferPutPtr = GPRS_EEP_BUFFER_START;
            g_eepromBufferGetPtr = GPRS_EEP_BUFFER_START;
            return;
        }
        /* Neu du lieu doc duoc tu EEPROM la ok (khac nhau khong qua 10 lan) */
        else
        {
            /* Nhan vi tri du lieu ghi tu EEPROM */
            g_eepromBufferPutPtr = tmp[0];
            /* Kiem tra neu vi tri du lieu nhan duoc = 0xFFFFFFFF (chuan bi tran), hoac < vi tri bat dau,
               hoac lon hon vi tri bat dau + 12000 */
            if (g_eepromBufferPutPtr == 0xFFFFFFFF ||
                g_eepromBufferPutPtr < GPRS_EEP_BUFFER_START ||
                g_eepromBufferPutPtr > GPRS_EEP_BUFFER_START + 12000)
            {
                /* Thiet lap vi tri du lieu ghi ve vi tri ban dau */
                g_eepromBufferPutPtr = GPRS_EEP_BUFFER_START;
                break;
            }
            else
            {
                break;
            }
        }
    }
    /* Thiet lap lai vi tri du lieu doc tu EEPROM */
    failCount = 0;
    while (1)
    {
        /* Xoa toan bo tmp */
        for (i = 0; i < 10; i++)
        {
            tmp[i] = 0;
        }
        /* Tien hanh doc 10 lan vi tri du lieu doc trong EEPROM */
        for (i = 0; i < 10; i++)
        {
            EE_Read(GPRS_FRAME_GET_PTR_EEP_ADDRESS, &tmp[i], 4);
            Delay(50);
            if (i > 0)
            {
                /* Kiem tra neu du lieu vua doc duoc khac lan doc truoc */
                if (tmp[i] != tmp[i - 1])
                {
                    failCount++;        /* Dem so lan khac */
                    if (failCount > 10)
                    {
                        break;
                    }
                    i = 0;
                }
            }
        }
        /* Neu du lieu qua moi lan doc khac nhau qua 10 lan*/
        if (failCount > 10)
        {
            /* Thiet lap toan bo vi tri du lieu doc va ghi vi tri ban dau */
            g_eepromBufferPutPtr = GPRS_EEP_BUFFER_START;
            g_eepromBufferGetPtr = GPRS_EEP_BUFFER_START;
            return;
        }
        /* Neu du lieu doc duoc tu EEPROM la ok (khac nhau khong qua 10 lan)*/
        else
        {
            /* Nhan vi tri du lieu doc tu EEPROM */
            g_eepromBufferGetPtr = tmp[0];
            /* Kiem tra neu vi tri du lieu nhan duoc = 0xFFFFFFFF (chuan bi tran), hoac < vi tri bat dau,
               hoac lon hon vi tri bat dau + 12000 (vi tri gioi han) */
            if (g_eepromBufferGetPtr == 0xFFFFFFFF ||
                g_eepromBufferGetPtr < GPRS_EEP_BUFFER_START ||
                g_eepromBufferGetPtr > GPRS_EEP_BUFFER_START + 12000)
            {
                /* Thiet lap vi tri du lieu doc ve vi tri ban dau */
                g_eepromBufferGetPtr = GPRS_EEP_BUFFER_START;
                break;
            }
            else
            {
                break;
            }
        }
    }
}
/**
 * @brief   Ham cau hinh chan I/O va khoi tao SIM
 *
 * @param   NONE
 * @retval  NONE
 */
void Gsm_Init()
{
    Pin_Init(GSM_PW_PIN, PIN_OUTPUT);           /* Khoi tao chan cap nguon, dau ra*/
    Pin_Init(GSM_RST_PIN, PIN_OUTPUT);          /* Khoi tao chan reset, dau ra*/
    Pin_Init(GSM_STS_PIN, PIN_INPUT);           /* Khoi tao chan status, dau ra*/
    Pin_SetMux(GSM_RX_PIN, PIN_MUX_ALT3);       /* Khoi tao chan nhan du lieu UART */
    Pin_SetMux(GSM_TX_PIN, PIN_MUX_ALT3);       /* Khoi tao chan truyen du lieu UART */

    /* Enable pullup resistor for rx pin */
    PORTD->PCR[17] |= PORT_PCR_PE_MASK;
    PORTD->PCR[17] |= PORT_PCR_PS_MASK;

    g_serial.port = 2;                          /* Cong UART2 */
    g_serial.baudrate = 115200;                 /* Toc do Baud 9600 */
    g_serial.privateObj.useRxBuffer = true;
    g_serial.rxBuffer = &g_uartBuffer[0];       /* Bo dem nhan UART*/
    g_serial.rxBufferSize = 300;                /* Kich thuoc bo dem nhan */

    Serial_Init(&g_serial, g_serial.baudrate);  /* Khoi tao bo UART2 */

    Pin_Write(GSM_PW_PIN, 1);                   /* Kich hoat chan cap nguon 4V cho module SIM */
    Pin_Write(GSM_RST_PIN, 1);                  /* Keo chan RST module SIM xuong 0V -> cho phep hoat dong */

    Gsm_TurnOff();                              /* Tat nguon SIM */
    Delay(500);
    Gsm_TurnOn();                               /* Bat nguon SIM -> Khoi dong lai*/

    uint32_t count = 0;
    while(count < 15)
    {
      Delay(1000);
      count++;
      Wdog_Refesh();
    }

    g_periodSendingTick = GetTickCount();
    Serial_Write(&g_serial, "ATE0\n\r", 6);     /* ATE0 -> Tat che so Echo (phan hoi ca lenh) */
    Delay(1000);

    sprintf(strGsm, "AT+CLVT=%d\n\r", Perh_GetCallVolume());               /* Lenh sai, phai la AT+CLVL -> Set volume loa*/
    Serial_Write(&g_serial, strGsm, strlen(strGsm));
    Delay(1000);
    sprintf(strGsm, "AT+CMIC=0,%d\n\r", Perh_GetMicrophoneLevel());        /* Lenh set do khech dai MIC */
    Serial_Write(&g_serial, strGsm, strlen(strGsm));
    Delay(1000);

    /* detect sim card */
    DetectSimCard();

    RestoreEepBufferPointer();                  /* Thiet lap vi tri du lieu ghi, doc trong ROM */

    sprintf(bufferDebug, "Gsm >> GPRS frame put pointer: %d", g_eepromBufferPutPtr);
    Dbg_Println(bufferDebug);

    sprintf(bufferDebug, "Gsm >> GPRS frame get pointer: %d", g_eepromBufferGetPtr);
    Dbg_Println(bufferDebug);
}
/**
 * @brief   Ham bat nguon 4V cung cap cho module SIM
 *
 * @param   NONE
 * @retval  NONE
 */
void Gsm_TurnOn()
{
    Pin_Write(GSM_PW_PIN, 0);
}
/**
 * @brief   Ham tat nguon 4V cung cap cho module SIM
 *
 * @param   NONE
 * @retval  NONE
 */
void Gsm_TurnOff()
{
    Pin_Write(GSM_PW_PIN, 1);
}
/**
 * @brief   Ham cai dat volume cho dau ra loa tren module SIM
 *
 * @param   uint8_t value -> Am luong cai dat (value = 0 ~ 10)
 * @retval  NONE
 */
void Gsm_SetAudioLevel(uint8_t value)
{
    if (value > 10)
    {
        value = 10;
    }

    sprintf(strGsm, "AT+CLVL=%d\n\r", value * 10);         /* Lenh AT cai dat am luong ra loa */
    Serial_Write(&g_serial, strGsm, strlen(strGsm));
    Delay(200);
}
/**
 * @brief   Ham cai dat do khuech dai cho MIC (volume MIC)
 *
 * @param   uint8_t value -> Am luong cai dat (value = 0 ~ 10)
 * @retval  NONE
 */
void Gsm_SetMicrophoneLevel(uint8_t value)
{
    if (value > 10)
    {
        value = 10;
    }
    value += 5;

    sprintf(strGsm, "AT+CMIC=0,%d\n\r", value);            /* Lenh AT cai dai do khuech dai cho MIC */
    Serial_Write(&g_serial, strGsm, strlen(strGsm));
    Delay(200);
}
/**
 * @brief   Ham luu truy du lieu cua frame truyen vao EEPROM
 *
 * @param   uint8_t* data -> Con tro, tro den dia chi du lieu can ghi
 * @param   uint32_t length -> Do dai du lieu can ghi
 * @retval  NONE
 */
static void SaveFrameToEeprom(uint8_t* data, uint32_t length)
{
    uint8_t tmp = 0, failCount = 0;
    uint8_t tmpArray[200];
    int i = 0;
    /* Neu frame la ban tin dong cua, data[20] = 0x64 'd' */
    if (data[20] == 'd')
    {
        /* Luu do dai cua frame truyen la 140, length - 26 */
        EE_WriteByte(GPRS_FRAME_EEP_ADDRESS, length - 26);
        Delay(1);
        /* Kiem tra lai lenh ghi */
        while (1)
        {
            tmp = 0;
            /* Doc lai du lieu vua ghi */
            EE_Read(GPRS_FRAME_EEP_ADDRESS, &tmp, 1);
            if (tmp == length - 26)     /* Neu du lieu doc duoc = du lieu ghi vao*/
            {
                break;                  /* Thoat */
            }
            else
            {
                /* Dem so lan ghi loi */
                failCount++;
                if (failCount > 50)     /* Neu ghi loi qua 50 lan -> thoat */
                {
                    return;
                }
                /* Tien hanh ghi lai du lieu */
                EE_WriteByte(GPRS_FRAME_EEP_ADDRESS, length - 26);
            }
        }

        /* Ghi du lieu cua frame dong cua, ghi tu dia chi tiep theo (sau dia chi ghi do dai) */
        EE_Write(GPRS_FRAME_EEP_ADDRESS + 1, &data[20], length - 26);
        Delay(1);
        /* Kiem tra lai lenh ghi */
        while (1)
        {
            /* Xoa toan bo mang tmpArray */
            for (i = 0; i < 200; i++)
            {
                tmpArray[i] = 0;
            }
            /* Doc lai toan bo du lieu vua ghi vao mang tmpArray */
            EE_Read(GPRS_FRAME_EEP_ADDRESS + 1, tmpArray, length - 26);
            for (i = 0; i < length - 26; i++)
            {
                /* So sanh tung byte doc duoc voi tung bai ghi vao */
                if (tmpArray[i] != data[20 + i])
                {
                    Delay(5);
                    /* Neu khac nhau (ghi loi) tien hanh ghi lai */
                    EE_Write(GPRS_FRAME_EEP_ADDRESS + 1, &data[20], length - 26);
                    break;
                }
            }
            if (i == length - 26)
            {
                break;
            }
        }
    }
    /* Neu la Frame cac ban tin khac ngoai ban tin dong cua */
    else
    {
        /* Ghi do dai cua du lieu vao con tro vi tri du lieu trong ROM  */
        EE_WriteByte(g_eepromBufferPutPtr, length - 26);
        /* Kiem tra lai lenh ghi */
        while (1)
        {
            tmp = 0;
            /* Doc lai du lieu vua ghi vao tmp */
            EE_Read(g_eepromBufferPutPtr, &tmp, 1);
            /* So sanh neu du lieu giong nhau -> thoat */
            if (tmp == length - 26)
            {
                break;
            }
            /* Neu du lieu khac nhau */
            else
            {
                failCount++;
                if (failCount > 50)     /* Neu so lan ghi loi > 50 -> thoat */
                {
                    return;
                }
                /* Tien hanh ghi lai du lieu */
                EE_WriteByte(g_eepromBufferPutPtr, length - 26);
            }
        }
        /* Luu du lieu cua ban tin khac ngoai ban tin dong cua */
        EE_Write(g_eepromBufferPutPtr + 1, &data[20], length - 26);

        Delay(1);
        /* Kiem tra lai lenh ghi */
        while (1)
        {
            /* Xoa toan bo mang tmpArray */
            for (i = 0; i < 200; i++)
            {
                tmpArray[i] = 0;
            }
            /* Doc lai toan bo du lieu vua ghi vao mang tmpArray */
            EE_Read(g_eepromBufferPutPtr + 1, tmpArray, length - 26);
            for (i = 0; i < length - 26; i++)
            {
                /* So sanh tung byte doc duoc voi tung bai ghi vao */
                if (tmpArray[i] != data[20 + i])
                {
                    Delay(5);
                    /* Neu khac nhau (ghi loi) tien hanh ghi lai */
                    EE_Write(g_eepromBufferPutPtr + 1, &data[20], length - 26);
                    break;
                }
            }
            if (i == length - 26)
            {
                break;
            }
        }

        /* Tu dong tang con tro ghi len 1 khung du lieu (10 byte) */
        g_eepromBufferPutPtr += 11;
        /* Neu con tro ghi vuot qua gioi han -> quay lai vi tri dau */
        if (g_eepromBufferPutPtr >= GPRS_EEP_BUFFER_END)
        {
            g_eepromBufferPutPtr = GPRS_EEP_BUFFER_START;
        }

        /* Neu bo dem day (con tro ghi = con tro doc), loai bo cac khung du lieu
           truoc do bang cach cap nhat lai con tro doc du lieu (get)*/
        if (g_eepromBufferPutPtr == g_eepromBufferGetPtr)
        {
            /* Tu dong tang con tro doc len 1 khung du lieu (10 byte) */
            g_eepromBufferGetPtr += 11;
            /* Neu con tro doc vuot qua gioi han -> quay lai vi tri dau */
            if (g_eepromBufferGetPtr >= GPRS_EEP_BUFFER_END)
            {
                g_eepromBufferGetPtr = GPRS_EEP_BUFFER_START;
            }
            /* Luu vi tri du lieu (con tro doc, ghi) vao EEPROM */
            SaveEepBufferGetPointer();
        }
        /* Luu vi tri du lieu (con tro doc, ghi) vao EEPROM */
        SaveEepBufferPutPointer();
    }
}
/**
 * @brief   Ham kiem tra du lieu da co trong EEPROM, da tao FRAME
 *
 * @param   NONE
 * @retval  uint8_t GetCloseDoorFrame -> Do dai cua Frame truyen
 */
static bool EepromHasFrame()
{
    /* Kiem tra neu vi tri du lieu doc khac vi tri du lieu ghi*/
    if (g_eepromBufferGetPtr != g_eepromBufferPutPtr)
    {
        return true;    /* Tra ket qua da co Frame */
    }

    uint8_t length = 0xff;
    /* Hoac doc trong ROM co bao Frame dong cua */
    EE_Read(GPRS_FRAME_EEP_ADDRESS, &length, 1);
    if (length == 140)
    {
        return true;    /* Tra ket qua da co Frame */
    }
    return false;
}
/**
 * @brief   Ham doc du lieu tu ROM va xay dung frame dong cua
 *
 * @param   uint8_t* data -> Con tro, tro den bo dem truyen ( dataToSend )
 * @retval  uint8_t GetCloseDoorFrame -> Do dai cua Frame truyen
 */
static uint8_t GetCloseDoorFrame(uint8_t* data)
{
    int i = 0;
    uint8_t frameSize = 0;
    /* Lay do dai cua Frame dong cua luu trong EEPROM */
    EE_Read(GPRS_FRAME_EEP_ADDRESS, &frameSize, 1);
    /* Neu do dai Frame = 140 */
    if (frameSize == 140)
    {
        /* Doc du lieu luu trong EEPROM bat dau tu 2008 den 2148, luu vao data tu data[20] den data [160] */
        EE_Read(GPRS_FRAME_EEP_ADDRESS + 1, &data[20], frameSize);
        /* Neu du lieu doc ROM dung */
        if (data[20] == 'd')
        {
            /* Nhap cac ki thu theo frame dong cua quy dinh vao data[0] -> data[19]
            Frame = ###|TPA0123456|CMD=|d|....|CRC_H,CRC_L&&& */
            data[0] = '#'; data[1] = '#'; data[2] = '#'; data[3] = '|';
            data[4] = 'T'; data[5] = 'P'; data[6] = 'A'; data[7] = '0';
            /* Doc ID may trong ROM la so tu nhien */
            Perh_GetVendId(&data[8]);
            for (i = 8; i < 14; i++)
            {
                /* Lay ID + 0x31 ('0') de chuyen thanh ki tu */
                data[i] += '0';
            }
            data[14] = '|'; data[15] = 'C'; data[16] = 'M'; data[17] = 'D';
            data[18] = '='; data[19] = '|';
            data[20 + frameSize] = '|';
            data[20 + frameSize + 3] = '&'; data[20 + frameSize + 4] = '&';
            data[20 + frameSize + 5] = '&';
            /* Do dai Frame duoc cong them 26 = 166 byte */
            frameSize = frameSize + 26;
            /* Tinh CRC */
            uint16_t crc = CRC_XModem(&data[3], frameSize - 8);
            data[frameSize - 5] = (uint8_t)(crc >> 8);          /* Dua CRC byte cao vao byte 161*/
            data[frameSize - 4] = (uint8_t)crc;                 /* Dua CRC byte thap vao byte 162*/
            /* Tra lai do dai Frame */
            return frameSize;
        }
        /* Neu du lieu doc ROM sai */
        else
        {
            Dbg_Println("Gsm >> CloseDoor Frame in eeprom is error");
        }
    }
    return 0;
}
/**
 * @brief   Ham lay du lieu tu ROM va xay dung Frame truyen (Doi voi frame OpenDoor/ReleasedItem/PaidoutChange/SellingError/DeviceError)
 *
 * @param   uint8_t* data -> Con tro, tro den bo dem truyen ( dataToSend )
 * @retval  static uint8_t GetFrameFromEeprom -> Do dai cua Frame truyen
 */
static uint8_t GetFrameFromEeprom(uint8_t* data)
{
    int i = 0;
    uint8_t frameSize = 0;
    /* Kiem tra vi tri du lieu doc ra va vi tri du lieu ghi vao ROM dang khac nhau (Ring Buffer) */
    if (g_eepromBufferGetPtr != g_eepromBufferPutPtr)
    {
        /* Lay do dai du lieu can truyen duoc luu trong ROM, tai dia chi g_eepromBufferGetPtr */
        EE_Read(g_eepromBufferGetPtr, &frameSize, 1);
        /* Neu do do nam trong khoang 4 ~ 10 byte */
        if (frameSize <= 10 && frameSize >= 4)
        {
            /* Lay du lieu can truyen trong ROM bat dau tu vi tri g_eepromBufferGetPtr + 1 */
            EE_Read(g_eepromBufferGetPtr + 1, &data[20], frameSize);
            /* Neu la cac frame OpenDoor/ReleasedItem/PaidoutChange/SellingError/DeviceError*/
            if (data[20] == 'c' || data[20] == 'e' || data[20] == 'i' || data[20] == 'f' || data[20] == 'g')
            {
            /* Frame = ###|TPA0123456|CMD=|c,e,i,f,g|....|CRC_H,CRC_L&&& */
                data[0] = '#'; data[1] = '#'; data[2] = '#'; data[3] = '|';
                data[4] = 'T'; data[5] = 'P'; data[6] = 'A'; data[7] = '0';
                /* Doc ID may trong ROM la so tu nhien */
                Perh_GetVendId(&data[8]);
                for (i = 8; i < 14; i++)
                {
                    /* Lay ID + 0x31 ('0') de chuyen thanh ki tu */
                    data[i] += '0';
                }
                data[14] = '|'; data[15] = 'C'; data[16] = 'M'; data[17] = 'D';
                data[18] = '='; data[19] = '|';
                data[20 + frameSize] = '|';
                data[20 + frameSize + 3] = '&'; data[20 + frameSize + 4] = '&';
                data[20 + frameSize + 5] = '&';

                frameSize = frameSize + 26;
                /* Tinh CRC */
                uint16_t crc = CRC_XModem(&data[3], frameSize - 8);
                data[frameSize - 5] = (uint8_t)(crc >> 8);      /* Dua CRC byte cao */
                data[frameSize - 4] = (uint8_t)crc;             /* Dua CRC byte thap */
                return frameSize;
            }
            else
            {
                Dbg_Println("Gsm >> Frame in eeprom is error message");
                Dbg_Println("Gsm >> Fix point get and put");
                g_eepromBufferGetPtr = g_eepromBufferPutPtr;
            }
        }
    }
    return 0;
}
/**
 * @brief   Ham kiem tra da co Frame de gui
 * @param   NONE
 * @retval  static uint8_t IsThereFrameToSend -> Trang thai frame
            (1 = Co frame trong ROM, 2 = Co Frame trong Ram, 0 = khong co frame)
 */
static uint8_t IsThereFrameToSend()
{
    if (EepromHasFrame())
    {
        return 1;
    }
    else if (g_frameBufferOnRamGetPtr != g_frameBufferOnRamGetPtr)
    {
        return 2;
    }
    return 0;
}
/**
 * @brief   Ham gui du lieu vao trong ROM hoac RAM tai trinh xu ly GSM
 * @param   gprs_server_type_t serverType -> Bien khong su dung
 * @param   uint8_t* data -> Con tro, tro den du lieu can ghi
 * @param   uint32_t length -> Do dai cua du lieu
 * @retval  NONE
 */
void Gsm_SendData(gprs_server_type_t serverType, uint8_t* data, uint32_t length)
{
    /* Kiem tra neu trang thai ROM dang loi*/
    if (g_eepromIsError == true)
    {
        Dbg_Println("Gsm >> EEPROM is error");
    }
    /* Neu ROM dang loi, hoac ban tin gui du lieu theo chu ki (0x62), hoac ban tin cap nhat thoi gian thuc (0x70),
       hoac ban tin bao loi thiet bi (0x67) voi loi EEPROM khong su dung duoc */
    if ((g_eepromIsError == true) || data[20] == 0x62 || data[20] == 0x70 || (data[20] == 0x67) && (data[22] == 0x70))
    {
        /* Luu tat ca frame truyen va do dai vao trong RAM */
        for (int i = 0; i < length; i++)
        {
            g_frameBufferOnRam[g_frameBufferOnRamPutPtr].dataToSend[i] = data[i];
        }
        g_frameBufferOnRam[g_frameBufferOnRamPutPtr].length = length;

        g_frameBufferOnRamPutPtr++;
        if (g_frameBufferOnRamPutPtr >= GPRS_FRAME_ON_RAM_SIZE)
        {
            g_frameBufferOnRamPutPtr = 0;
        }
    }
    /* Neu ROM khong loi, hoac cac khung truyen khong quan trong duoc luu vao ROM */
    else
    {
        SaveFrameToEeprom(data, length);
    }
}

/**
 * @brief   Ham tao frame luu du lieu bao mo cua vao ROM hoac RAM
 * @param   NONE
 * @retval  NONE
 */
void Gsm_SendOpenDoorFrame()
{
    int hour = 0, minute = 0, second = 0, date = 0, month = 0, year = 0;
    /* Cap nhat thoi gian thuc tu RTC */
    Rtc_GetCurrentTime(&hour, &minute, &second);
    Rtc_GetCurrentDate(&date, &month, &year);
    /* Dua cac du lieu thoi gian thuc vao frame truyen */
    g_FrameOpenDoor[22] = year;
    g_FrameOpenDoor[23] = month;
    g_FrameOpenDoor[24] = date;
    g_FrameOpenDoor[25] = hour;
    g_FrameOpenDoor[26] = minute;
    g_FrameOpenDoor[27] = second;
    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameOpenDoor[3], 26);
    g_FrameOpenDoor[29] = (uint8_t)(crc >> 8);
    g_FrameOpenDoor[30] = (uint8_t)crc;
    /* Luu tru du lieu va ROM hoac RAM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameOpenDoor, 34);
}
/**
 * @brief   Ham tao frame bao dong cua, luu du lieu bao dong cua vao ROM
 * @param   NONE
 * @retval  NONE
 */
void Gsm_SendCloseDoorFrame()
{
    int hour = 0, minute = 0, second = 0, date = 0, month = 0, year = 18;

    /* Kiem tra trang thai led chieu sang -> dua vao byte dia chi 22 */
    if (Perh_HeaterIsOn())
    {
        g_FrameCloseDoor[22] = '1';
    }
    else
    {
        g_FrameCloseDoor[22] = '0';
    }

    /* Kiem tra do am cai dat -> dua vao byte dia chi 23 */
    g_FrameCloseDoor[23] = Perh_GetHumidity();

    /* Kiem tra trang thai cam bien roi -> dua vao byte dia chi 24 */
    if (Perh_DropSensorIsOn())
    {
        g_FrameCloseDoor[24] = '1';
    }
    else
    {
        g_FrameCloseDoor[24] = '0';
    }

    /* Kiem tra trang thai dau doc tien */
    /* Neu su kien gan nhat la dau doc tien loi */
    if (NV11_GetLatestEvent() == NV11_ERROR)
    {
        /* Xoa toan bo du lieu tu dia chi 25- 33 ve 0 */
        for (int channelIndex = 0; channelIndex < 9; channelIndex++)
        {
            g_FrameCloseDoor[25 + channelIndex] = 0;   /* The number of change notes */
        }
    }
    /* Neu la cac su kien khac */
    else
    {
        /* Xoa toan bo du lieu tu dia chi 25- 33 ve 0 */
        for (int channelIndex = 0; channelIndex < 9; channelIndex++)
        {
            g_FrameCloseDoor[25 + channelIndex] = 0;
        }
        /* Neu la dau doc NV11, byte dia chi 27 se chua so to tien duoc luu tru trong Floatnote = tong so tien/ menh gia */
    /* byte |  25 |  26  |  27  |   28  |   29  |  30   |   31  |   32   |    33  | */
    /* Note | 1000| 2000 | 5000 | 10000 | 20000 | 50000 | 100000| 200000 | 500000 | */
        if (NV11_GetUnitType() == NV_TYPE_NV11)
        {
            /* Tinh so to tien tren FloatNote (tra lai) luu vao byte dia chi 27 */
        uint8_t noteNumber = NV11_GetAvailableChange() / NV11_GetDenominationForChange();
        switch(NV11_GetDenominationForChange())
        {
            case 1000:
          g_FrameCloseDoor[25] = noteNumber;
          break;
        case 2000:
          g_FrameCloseDoor[26] = noteNumber;
          break;
        case 5000:
          g_FrameCloseDoor[27] = noteNumber;
          break;
        case 10000:
          g_FrameCloseDoor[28] = noteNumber;
          break;
        case 20000:
          g_FrameCloseDoor[29] = noteNumber;
          break;
        case 50000:
          g_FrameCloseDoor[30] = noteNumber;
          break;
        case 100000:
          g_FrameCloseDoor[31] = noteNumber;
          break;
        case 200000:
          g_FrameCloseDoor[32] = noteNumber;
          break;
        case 500000:
          g_FrameCloseDoor[33] = noteNumber;
          break;
        }
        }
        /* Neu la dau doc NV200, byte tu 25 - 33 se luu tru so to tien tung menh gia trong Floatnote */
        else if (NV11_GetUnitType() == NV_TYPE_NV200)
        {
            for (int channelIndex = 0; channelIndex < 9; channelIndex++)
            {
                g_FrameCloseDoor[25 + channelIndex] = NV11_GetStoredNoteByChannel(channelIndex);   /* The number of change notes */
            }
        }
    }
    /* Luu tru gia cua san pham tai moi ngan */
    for (int i = 34; i < 94; i++)
    {
        g_FrameCloseDoor[i] = 0x00;//Perh_GetItemPrice(i - 33) / 1000;
    }

    /* Luu tru so luong san pham tren moi ngan */
    for (int i = 94; i < 154; i++)
    {
        g_FrameCloseDoor[i] = 0xFF;//Perh_GetItemNumber(i - 93);
    }

    /* Lay thoi gian thuc tu RTC */
    Rtc_GetCurrentTime(&hour, &minute, &second);
    Rtc_GetCurrentDate(&date, &month, &year);
    /* Luu tru thoi gian thuc vao frame */
    g_FrameCloseDoor[154] = year;
    g_FrameCloseDoor[155] = month;
    g_FrameCloseDoor[156] = date;
    g_FrameCloseDoor[157] = hour;
    g_FrameCloseDoor[158] = minute;
    g_FrameCloseDoor[159] = second;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameCloseDoor[3], 158);
    g_FrameCloseDoor[161] = (uint8_t)(crc >> 8);
    g_FrameCloseDoor[162] = (uint8_t)crc;

    /* Luu tru du lieu dong cua  */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameCloseDoor, 166);
}
/**
 * @brief   Ham tao frame bao khi co khach mua hang va luu tru du lieu EEPROM
 * @param   uint8_t receivedNote -> Gia tri tien da nhan
 * @param   uint8_t selectedSlot -> Khay san pham khach hang lua chon
 * @retval  NONE
 */
void SendReleasedItemFrame(uint8_t receivedNote, uint8_t selectedSlot)
{
    int hour = 0, minute = 0, second = 0, date = 0, month = 0, year = 18;
    /* Luu tru so tien da nhan vao byte dia chi 22*/
    g_FrameReleasedItem[22] = receivedNote;
    /* Luu tru san pham khach chon vao byte dia chi 22*/
    g_FrameReleasedItem[23] = selectedSlot;

    /* Lay du lieu thoi gian thuc trong ROM  */
    Rtc_GetCurrentTime(&hour, &minute, &second);
    Rtc_GetCurrentDate(&date, &month, &year);
    g_FrameReleasedItem[24] = year;
    g_FrameReleasedItem[25] = month;
    g_FrameReleasedItem[26] = date;
    g_FrameReleasedItem[27] = hour;
    g_FrameReleasedItem[28] = minute;
    g_FrameReleasedItem[29] = second;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameReleasedItem[3], 28);
    g_FrameReleasedItem[31] = (uint8_t)(crc >> 8);
    g_FrameReleasedItem[32] = (uint8_t)crc;

    /* Ghi du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameReleasedItem, 36);
}
/**
 * @brief   Ham tao frame bao khi tra lai tien va luu tru du lieu EEPROM
 * @param   uint8_t payoutNote -> Gia tri tien tra lai
 * @retval  NONE
 */
void SendPayoutChangeFrame(uint8_t payoutNote)
{
    int hour = 0, minute = 0, second = 0, date = 0, month = 0, year = 18;
    /* Luu tru so tien tra lai vao byte dia chi 22*/
    g_FramePaidoutChange[22] = payoutNote;

    /* Lay du lieu thoi gian thuc trong ROM  */
    Rtc_GetCurrentTime(&hour, &minute, &second);
    Rtc_GetCurrentDate(&date, &month, &year);
    g_FramePaidoutChange[23] = year;
    g_FramePaidoutChange[24] = month;
    g_FramePaidoutChange[25] = date;
    g_FramePaidoutChange[26] = hour;
    g_FramePaidoutChange[27] = minute;
    g_FramePaidoutChange[28] = second;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FramePaidoutChange[3], 27);
    g_FramePaidoutChange[30] = (uint8_t)(crc >> 8);
    g_FramePaidoutChange[31] = (uint8_t)crc;

    /* Ghi du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FramePaidoutChange, 35);
}

/**
 * @brief   Ham tao frame bao mua hang loi va luu tru du lieu EEPROM
 * @param   uint8_t receivedNote -> Gia tri tien da nhan
 * @param   uint8_t errorSlot -> Vi tri khay hang bi loi
 * @retval  NONE
 */
void SendSellingErrorFrame(uint8_t receivedNote, uint8_t errorSlot)
{
    int hour = 0, minute = 0, second = 0, date = 0, month = 0, year = 18;
    /* Luu tru so tien da nhan vao byte dia chi 22*/
    g_FrameSellingError[22] = receivedNote;
    /* Luu tru vi tri khay hang loi byte dia chi 23*/
    g_FrameSellingError[23] = errorSlot;

    /* Lay du lieu thoi gian thuc trong ROM  */
    Rtc_GetCurrentTime(&hour, &minute, &second);
    Rtc_GetCurrentDate(&date, &month, &year);
    g_FrameSellingError[24] = year;
    g_FrameSellingError[25] = month;
    g_FrameSellingError[26] = date;
    g_FrameSellingError[27] = hour;
    g_FrameSellingError[28] = minute;
    g_FrameSellingError[29] = second;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameSellingError[3], 28);
    g_FrameSellingError[31] = (uint8_t)(crc >> 8);
    g_FrameSellingError[32] = (uint8_t)crc;

    /* Luu du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameSellingError, 36);
}

/**
 * @brief   Ham tao frame bao loi dong co va luu tru du lieu EEPROM
 * @param   uint8_t motorId -> ID cua dong co loi
 * @retval  NONE
 */
void SendMotorErrorFrame(uint8_t motorId)
{
    /* Luu tru ID cua dong co loi vao byte dia chi 22*/
    g_FrameDeviceError[22] = motorId;
    /* Luu tinh trang dong co: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x30;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}

/**
 * @brief   Ham tao frame bao dong co da duoc sua loi va luu tru du lieu EEPROM
 * @param   NONE
 * @retval  NONE
 */
void SendMotorResumeFrame()
{
    /* Luu tru ID cua dong co loi vao byte dia chi 22*/
    g_FrameDeviceError[22] = 1;
    /* Luu tinh trang dong co: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x31;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu tru du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}

/**
 * @brief   Ham tao frame bao doc cam bien do am bi loi va luu tru du lieu EEPROM
 * @param   NONE
 * @retval  NONE
 */
static void SendSsHumidityError()
{
  /* Luu ma loi */
  g_FrameDeviceError[22] = 0x68;
  /* Luu tinh trang loi: 0x30 = Dang loi, 0x31 = Da sua loi */
  g_FrameDeviceError[23] = 0x30;

  /* Tinh CRC */
  uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
  g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
  g_FrameDeviceError[26] = (uint8_t)crc;

  /* Luu du lieu vao ROM */
  Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}

/**
 * @brief   Ham tao frame bao loi cam bien do am duoc sua loi va luu tru du lieu EEPROM
 * @param   NONE
 * @retval  NONE
 */
static void SendSsHumidityResume()
{
  /* Luu ma loi */
  g_FrameDeviceError[22] = 0x68;
  /* Luu tinh trang loi: 0x30 = Dang loi, 0x31 = Da sua loi */
  g_FrameDeviceError[23] = 0x31;

  /* Tinh CRC */
  uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
  g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
  g_FrameDeviceError[26] = (uint8_t)crc;

  /* Luu du lieu vao ROM */
  Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}
/**
 * @brief   Ham kiem tra trang thai cua cam bien do am gui Server
 * @param   NONE
 * @retval  NONE
 */
void CheckSsHumidityStatus()
{
//  static uint8_t ssTemStatusErrorFlag = 0x02; /* 0x00 - khong co loi   0x01 - co loi    0x02 - khong xac dinh */
//  static uint8_t countError = 0;
//  static uint8_t countOk = 0;
//  static uint32_t oldTick = 0;
//  if(GetTickCount() - oldTick > 3000)
//  {
////    if(g_gsmHumidityValueCheckErr < 0 || g_gsmHumidityValueCheckErr > 30)
////    {
////      countError++;
////      /* Neu nhiet do vuot nguong qua 5 lan va truoc do khong co loi */
////      if(countError > 5 && ssTemStatusErrorFlag != 0x01)
////      {
////        /* Gui frame canh bam len server */
////        ssTemStatusErrorFlag = 0x01;
////        countOk = 0;
////        SendSsHumidityError();
////      }
////    }
////    else
////    {
////      countOk++;
////      /* Neu nhiet do ko vuot nguong 5 lan va truoc do co loi */
////      if(countOk > 5 && ssTemStatusErrorFlag != 0x00)
////      {
////        ssTemStatusErrorFlag = 0x00;
////        countError = 0;
////        SendSsHumidityResume();
////      }
////    }
//  }
}
/**
 * @brief   Ham kiem tra trang thai cua EEPROM va RTC, gui Server
 * @param   NONE
 * @retval  NONE
 */
void CheckEEStatus()
{
    int i = 0;
    static uint32_t oldTick = 0;
    uint8_t dummy;
    int hour, minute, second;
    /* Kiem tra trang thai EEPROM va RTC sau moi 2s */
    if (GetTickCount() - oldTick > 2000)
    {
        oldTick = GetTickCount();
        /* Kiem tra trang thai cua EEPROM */
        for (i = 0; i < 5; i++)
        {
            /* Tien hanh thu doc EEPROM dia chi 0 trong vong 5 lan (i = 0 ~ 4)*/
            if (EE_ReadByte(0, &dummy) == true)
            {
                /* Neu doc EEPROM la ok, va co bao loi EEPROM dang thiet lap */
                if (g_eepromIsError == true)
                {
                    /* Gui frame bao len server EEPROM da chay */
                    SendEepromResumeFrame();
                }
                /* Xoa co bao loi EEPROM */
                g_eepromIsError = false;
                break;  /* Thoat */
            }
        }
        /* Neu kiem tra trong 5 lan doc ma EEPROM khong phai hoi */
        if (i == 5)
        {
            /* Neu co bao EEPROM dang khong bi loi */
            if (g_eepromIsError == false)
            {
                /* Gui frame bao len server EEPROM bi loi */
                SendEepromErrorFrame();
            }
            /* Set co bao loi EEPROM */
            g_eepromIsError = true;
        }

        /* Kiem tra trang thai RTC */
        for (i = 0; i < 5; i++)
        {
            /* Tien hanh doc du lieu gio, phut, giay tu RTC trong vong 5 lan */
            if (Rtc_GetCurrentTime(&hour, &minute, &second) == true)
            {
                /* Neu doc RTC la ok, va co bao loi RTC dang thiet lap */
                if (g_rtcIsError == true)
                {
                    /* Gui len Server bao RTC da chay */
                    SendRtcResumeFrame();
                }
                /* Xoa co bao loi RTC */
                g_rtcIsError = false;
                break;
            }
        }
        /* Neu kiem tra trong vong 5 lan ma RTC khong phan hoi */
        if (i == 5)
        {
            /* Neu co bao RTC dang khong bi loi */
            if (g_rtcIsError == false)
            {
                /* Gui frame bao len server RTC bi loi */
                SendRtcErrorFrame();
            }
            /* Set co bao loi RTC */
            g_rtcIsError = true;
        }
    }
}
/**
 * @brief   Ham xu ly chinh GSM, gui du lieu len Server
 * @param   NONE
 * @retval  NONE
 */
void Gsm_Process()
{
    char str[30];
    static bool tmpFlag = false;
    /* Neu SIM thuc hien cuoc goi den tong dai */
    if (g_isCalling)
    {
        /* Thuc hien trinh xu ly cuoc goi */
        Gsm_CallProcess();
        return;
    }
    /* Cap nhat du lieu vao RAM sau moi 120s = 2 phut */
    if (GetTickCount() - g_periodSendingTick > 120000)
    {
        g_periodSendingTick = GetTickCount();
        Gsm_SendDataPeriod();   /* Luu du lieu dinh ki vao RAM */
    }
    /* Luu du lieu trang thai NV11 vao ROM */
    SendNV11ErrorFrame();
    /* Luu du lieu ban tin cap nhat thoi gian thuc vao RAM */
    SendTimeRequestFrame();
    /* Kiem tra trang thai EEPROM va RTC dinh ki */
    CheckEEStatus();
    /* Kiem tra trang thai cam bien do am */
    CheckSsHumidityStatus();
    /* Kiem tra trang thai State Machine GSM */
    switch (g_state)
    {
    /* Gui du lieu len Server khi ROM hoac RAM bao co Frame */
    case 0:
        /* Kiem tra du lieu co trong ROM */
        if (EepromHasFrame())
        {
            /* Neu process timeout qua 3 lan */
            if (g_failureCount >= 3)
            {
                g_failureCount = 0;
                Dbg_Println("Gsm >> Resetting the GSM module");
                /* Chuyen trang thai may sang Reset GSM = 0xF0 */
                g_state = 0xF0;
            }
            /* Neu GSM la ok (process timeout < 3 lan)*/
            else
            {
                /* Kiem tra neu con tro doc khac con tro ghi du lieu vao ROM (Tuc van con du lieu phai gui )*/
                if (g_eepromBufferGetPtr != g_eepromBufferPutPtr)
                {
                    /* Sao chep frame tu EEPROM toi g_gprsCurrentFrame va chuyen sang buoc tiep theo de gui frame */
                    Dbg_Println("\n\rGsm >> Getting frame from eeprom");
                    sprintf(bufferDebug, "Gsm >> Get Pointer: %d", g_eepromBufferGetPtr);
                    Dbg_Println(bufferDebug);

                    /* Ghi vao file log = Put Pointer: */
                    sprintf(bufferDebug, "Gsm >> Put Pointer: %d", g_eepromBufferPutPtr);
                    Dbg_Println(bufferDebug);
                    /* Lay do dai du lieu va du lieu tu ROM dua vao g_gprsCurrentFrame */
                    g_gprsCurrentFrame.length = GetFrameFromEeprom(g_gprsCurrentFrame.dataToSend);
                    sprintf(bufferDebug, "Gsm >> Frame length: %d", g_gprsCurrentFrame.length);
                    Dbg_Println(bufferDebug);
                    /* Kiem tra neu do dai du lieu bang 0*/
                    if (g_gprsCurrentFrame.length == 0)
                    {
                        /* Chuyen sang trang thai cho 30s roi quay ve kiem tra du lieu ROM va RAM */
                        g_state = 10;
                        g_signalQuantity = 0;
                        g_curTick = GetTickCount();
                        break;
                    }
                }
                /* Neu con tro doc = con tro ghi (Het du lieu trong ROM, RAM) */
                else
                {
                    Dbg_Println("\n\rGsm >> Getting the close door frame");
                    /* Chuyen sang lay du lieu frame dong cua de gui */
                    g_gprsCurrentFrame.length = GetCloseDoorFrame(g_gprsCurrentFrame.dataToSend);

                    sprintf(bufferDebug, "Gsm >> Frame length: %d", g_gprsCurrentFrame.length);
                    Dbg_Println(bufferDebug);

                    /* Neu do dai du lieu = 0*/
                    if (g_gprsCurrentFrame.length == 0)
                    {
                        /* Chuyen sang trang thai cho 30s roi quay ve kiem tra du lieu ROM va RAM */
                        g_state = 10;
                        g_signalQuantity = 10;
                        break;
                    }
                }
                /* Xoa co bao, tuc frame se gui tren ROM */
                g_isFrameFromRam = false;
                /* Chuyen sang trang thai 1: Khoi tao Server */
                g_state = 1;
            }
        }
        /* Neu process timeout qua 3 lan */
        else if (g_failureCount >= 3)
        {
            g_failureCount = 0;
            /* Neu khong the gui khung du lieu (0x62 ban tin dinh ki) qua 3 lan,
               xoa bo khung du lieu va chuyen toi khung du lieu tiep theo */
            if (g_frameBufferOnRam[g_frameBufferOnRamGetPtr].dataToSend[20] == 0x62)
            {
                Dbg_Println("Gsm >> Cannot send the cycling frame. Removed it from queue");
                /* Chuyen toi khung du lieu tiep theo */
                g_frameBufferOnRamGetPtr++;
                /* Neu vuot qua gioi han tren RAM, quay tro ve doc tu dia chi 0 (bat dau)*/
                if (g_frameBufferOnRamGetPtr >= GPRS_FRAME_ON_RAM_SIZE)
                {
                    g_frameBufferOnRamGetPtr = 0;
                }
            }
            /* Neu khong phai ban tin du lieu dinh ki (0x62) */ /* xoa ban tin sua o day */
            else
            {
                Dbg_Println("Gsm >> Resetting the GSM module");
                /* Chuyen trang thai sang thuc hien Reset GSM */
                g_state = 0xF0;
            }
        }
        /* Kiem tra du lieu co trong RAM de chuan bi gui */
        /* Neu con tro doc va con tro ghi du lieu RAM khac nhau -> co du lieu can truyen */
        else if (g_frameBufferOnRamPutPtr != g_frameBufferOnRamGetPtr)
        {
            Dbg_Println("\n\rGsm >> Getting frame from RAM");
            /* Chuyen do dai du lieu toi g_gprsCurrentFrame va chuan bi gui frame len sever */
            g_gprsCurrentFrame.length = g_frameBufferOnRam[g_frameBufferOnRamGetPtr].length;
            /* Chuyen du lieu toi g_gprsCurrentFrame va chuan bi gui frame len sever */
            for (int i = 0; i < g_frameBufferOnRam[g_frameBufferOnRamGetPtr].length; i++)
            {
                g_gprsCurrentFrame.dataToSend[i] = g_frameBufferOnRam[g_frameBufferOnRamGetPtr].dataToSend[i];
            }
            /* Set co bao fram se gui tren RAM */
            g_isFrameFromRam = true;
            /* Chuyen sang trang thai 1: Khoi tao Server */
            g_state = 1;
        }
        break;
    /* Trang thai khoi tao Server, chuan bi gui du lieu */
    case 1:
        sprintf(bufferDebug, "Gsm >> Start send data to main server with frame type: 0x%x", g_gprsCurrentFrame.dataToSend[20]);
        Dbg_Println(bufferDebug);
        Dbg_Print("Gsm >> Server: ");
        Dbg_Print(g_serverIP);
        Dbg_Print("/");
        Dbg_Println(g_serverPort);
        /* Xoa toan bo bo dem nhan du lieu */
        Serial_ClearRxBuffer(&g_serial);
        /* Thuc hien lenh AT+CIPSTART: Lenh bat dau ket noi TCP */
        Serial_Write(&g_serial, "AT+CIPSTART=\"TCP\",\"", 19);
        /* Thuc hien cau hinh IP server */
        Serial_Write(&g_serial, g_serverIP, strlen(g_serverIP));
        Serial_Write(&g_serial, "\",\"", 3);
        /* Thuc hien cau hinh PORT server */
        Serial_Write(&g_serial, g_serverPort, strlen(g_serverPort));
        Serial_Write(&g_serial, "\"\n\r", 3);
        g_curTick = GetTickCount();
        /* Chuyen sang trang thai 2: Cho phan hoi ket noi tu Server */
        g_state = 2;
        break;
    /* Kiem tra phan hoi cua Server sau khi thuc hien cau hinh */
    case 2:
        /* Neu chua nhan duoc du lieu, tiep tuc doi nhan */
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu nhan duoc chuoi phan hoi "CONNECT OK" hoac "ALREADY CONNECT" */
        if (Serial_FindString(&g_serial, "CONNECT OK") != NULL)
        {
            /* Chuyen sang trang thai 3: Thuc hien lenh AT+CIPSEND (gui chuoi ki tu)*/
            g_state = 3;
            g_signalQuantity = 10;
        }
        else if (Serial_FindString(&g_serial, "ALREADY CONNECT") != NULL)
        {
            /* Chuyen sang trang thai 3: Thuc hien lenh AT+CIPSEND (gui chuoi ki tu)*/
            g_state = 3;
            g_signalQuantity = 10;
        }
        /* Neu khong nhan duoc phan hoi, hoac chuoi ki tu khong dung */
        else if (GetTickCount() - g_curTick > 30000)
        {
            g_signalQuantity = 0;
            /* Cho sau 30s, bao loi Server -> Ghi file " Log " */
            Dbg_Print("Gsm >> Cannot connect to server ");
            Dbg_Print(g_serverIP);
            Dbg_Print("/");
            Dbg_Println(g_serverPort);
            /* Tinh thoi gian va so lan Timeout */
            g_failureCount++;
            g_curTick = GetTickCount();
            /* Chuyen sang trang thai 10: Doi 30s sau do thu lai (trang thai 0) */
            g_state = 10;
            g_signalQuantity = 0;
        }
        break;
    /* Trang thai 3: Thuc hien AT+CIPSEND (gui chuoi ki tu) */
    case 3:
        sprintf(strGsm, "AT+CIPSEND=%d\n\r", g_gprsCurrentFrame.length);
        /* Gui lenh AT+CIPSEND va cho phan hoi */
        Serial_Write(&g_serial, strGsm, strlen(strGsm));
        g_curTick = GetTickCount();
        /* Chuyen sang trang thai 4: Doc phan hoi ">" de gui chuoi ki tu */
        g_state = 4;
        Serial_ClearRxBuffer(&g_serial);
        break;
    /* Trang thai 4: Doc phan hoi ">" de gui chuoi ki tu */
    case 4:
        /* Cho nhan duoc ki tu ">" */
        if (Serial_FindString(&g_serial, ">") != NULL)
        {
            /* Chuyen sang trang thai 5: Gui chuoi ki tu */
            g_state = 5;
        }
        /* Neu qua 3s khong nhan duoc phan hoi, hoac phan hoi sai */
        else if (GetTickCount() - g_curTick > 5000)
        {
            /* Tinh thoi gian va so lan Timeout */
            g_failureCount++;
            g_curTick = GetTickCount();
            /* Chuyen sang trang thai 10: Doi 30s sau do thu lai (trang thai 0) */
            g_state = 10;
            g_signalQuantity = 0;
        }
        break;
    /* Trang thai 5: Gui chuoi ki tu */
    case 5:
        /* Gui chuoi ki tu (du lieu) len server */
        Serial_ClearRxBuffer(&g_serial);
        Serial_Write(&g_serial, g_gprsCurrentFrame.dataToSend, g_gprsCurrentFrame.length);
        Serial_Write(&g_serial, "\n\r", 2);

        /* Ghi du lieu vua gui vao file "log"*/
        //char str[10];
        int i,j;
        memset(FrameToLog,0,600);
        Dbg_Println("Gsm >> Data: ");
        for (i = 0; i < g_gprsCurrentFrame.length; i++)
        {
            if(i < 22)FrameToLog[i] = g_gprsCurrentFrame.dataToSend[i];
            else
            {
                sprintf(strGsm, "%02x ", g_gprsCurrentFrame.dataToSend[i]);
                j = (i - 22)*2;
                FrameToLog[22 + j] = strGsm[0];
                FrameToLog[23 + j] = strGsm[1];
            }
        }
        Dbg_Println(FrameToLog);
        g_curTick = GetTickCount();

        tmpFlag = false;
        /* Chuyen sang trang thai 6: Doi phan hoi tu GSM va tu Server */
        g_state = 6;
        break;
    /* Trang thai 6: Doi phan hoi tu GSM va tu Server */
    case 6:

        /* Neu GSM phai hoi da gui du lieu, cho Server phan hoi */
        if (tmpFlag == false)
        {
            if (Serial_FindString(&g_serial, "SEND OK") != NULL)
            {
                tmpFlag = true;
                /* Ghi du lieu vua gui vao file "log"*/
                Dbg_Println("Gsm >> Send OK. Waiting for response...");
                g_curTick = GetTickCount();
                break;
            }
        }

        /* Tim kiem ban tin phan hoi tu server */
        if (Serial_FindString(&g_serial, "###") != NULL &&
                Serial_FindString(&g_serial, "&&&") != NULL)
        {
          if(GetTickCount() - g_curTick > 3000)
          {
            g_curTick = GetTickCount();
            /* reset response flag */
            response.success_flag = 0x00;
            response.update_flag = 0x00;
            Gsm_check_response(&response);
            /* cap nhat thoi gian */
            if(response.success_flag == RESPONSE_SUCCESS)
            {
              Dbg_Println("Gsm >> Server responded SUCCESS");
              Dbg_Print("Gsm >> Time from server: ");
              sprintf(bufferDebug, "%d:%d:%d, %d/%d/%d",response.time.hours,response.time.minute,response.time.seconds,
                      response.time.date,response.time.month,response.time.year);
              Dbg_Println(bufferDebug);

              if(response.time.year >= 19)
              {
                  if(write_time_process(response.time.date,response.time.month,response.time.year + 2000,
                                        response.time.hours,response.time.minute,response.time.seconds) == false)
                  {
                      Dbg_Println("Gsm >> Write time from server to RTC ERROR! ");
                  }
              }
              /* chuyen sang trang thai 7: kiem tra con frame hoac ngat ket noi */
              g_state = 7;
            }
            else if(response.success_flag == RESPONSE_ERROR)
            {
              /* Ghi du lieu vua gui vao file "log"*/
              Dbg_Println("Gsm >> Server responded SERVER_ERROR. Trying to send again...");
              f_close(&g_logFile);
              /* Tinh thoi gian va so lan Timeout */
              g_failureCount++;
              g_curTick = GetTickCount();
              /* Chuyen sang trang thai 10: Doi 30s sau do thu lai (trang thai 0) */
              g_state = 10;
              g_signalQuantity = 0;
            }

            if(response.update_flag == 0x01)
            {
              Dbg_Println("Gsm >> Server request UPDATE");
              g_lenUpdateFileName = 0;
              g_lenUpdateFileName = sprintf(g_updateFileName,"VM_%d.%d.%d.bin",response.version[0], response.version[1], response.version[2]);
              /* lay do dai ten file update */
              memset(g_updateLastFileName, 0 , 32);
              ReadVerifyDataFlag(LENGTH_FILE_UPDATE_NAME_ADDRESS, &g_lenUpdateLastFileName);
              if(g_lenUpdateLastFileName > 20) g_lenUpdateLastFileName = 20;
              /* doc tien file */
              ReadVerifyData(FILE_UPDATE_NAME_ADDRESS,  g_updateLastFileName, g_lenUpdateLastFileName);
              uint8_t fileUpdateErrorCRC = 0;
              ReadVerifyDataFlag(FILE_UPDATE_CRC_ERROR_ADDRESS, &fileUpdateErrorCRC);
              if((strcmp(g_updateLastFileName, g_updateFileName) != 0 || fileUpdateErrorCRC == 0x55) && ftp_count_down < 5) /* KIEM TRA VERSION HIEN TAI TRONG MAY CO KHAC VOI VERSION CAN CAP NHAT KHONG */
              {
                g_savestate = g_state;
                Dbg_Println("Gsm >> Start update processing");
                /* xoa firmware cu trong bo nho sim */
                Serial_ClearRxBuffer(&g_serial);
                uint8_t _length = sprintf(buf_str,"AT+FSLS=C:\\User\\FTP\\\r\n");
                Serial_Write(&g_serial, buf_str, _length);
                g_curTick = GetTickCount();
                /* chuyen trang thai sang Update firmware */
                g_state = STATE_FTP_GET_LASTFILE;
                Dbg_Println("Gsm >> Jump to STATE_FTP_GET_LASTFILE");
                g_ftpErrorCount = 0;
              }
            }
          }
        }

        /* Cho gui hoan thanh, doi trong trong vong 30s */
        if (GetTickCount() - g_curTick > 30000)
        {
            /* Neu qua 30s khong nhan duoc bat ki phan hoi nao, hoac phai hoi khong dung */
            /* Ghi du lieu vua gui vao file "log"*/
            Dbg_Println("Gsm >> No response from server after 30 seconds. Trying to send again...");
            /* Dong file */
            f_close(&g_logFile);
            /* Tinh thoi gian va so lan Timeout */
            g_failureCount++;
            g_curTick = GetTickCount();
            /* Chuyen sang trang thai 10: Doi 30s sau do thu lai (trang thai 0) */
            g_state = 10;
            g_signalQuantity = 0;
        }
        break;
    case 7:
        /* Kiem tra neu con du lieu phai gui, khong ngat ket noi */
        if (IsThereFrameToSend() != 0)
        {
            /* Chuyen sang trang thai 8: Cap nhat con tro doc va quay ve trang thai 0
               de gui tiep du lieu */
            Serial_Write(&g_serial, "AT+CSQ\n\r", 8);
            g_curTick = GetTickCount();
            Delay(500);
            g_state = 8;
        }
        /* Neu khong con gui lieu phai gui */
        else
        {
            /* Ngat ket noi toi Server */
            Serial_Write(&g_serial, "AT+CIPCLOSE\n\r", 13);
            Delay(500);
            /* Signal quantity read */
            Serial_Write(&g_serial, "AT+CSQ\n\r", 8);
            g_curTick = GetTickCount();
            Delay(500);
            /* Chuyen sang trang thai 8: Cap nhat con tro doc va quay ve trang thai 0
               de gui tiep du lieu */
            g_state = 8;
        }
        break;
    /* Trang thai 8: Cap nhat signal quantity */
    case 8:
      if(Serial_FindString(&g_serial, "+CSQ:") != NULL)
      {
        /* Lay du lieu */
        uint8_t pData = ((uint8_t)((uint8_t*)Serial_FindString(&g_serial, "+CSQ:") - g_uartBuffer)) + 6;
        uint8_t bufSignal = 0;
        if(g_uartBuffer[pData]     != ',') bufSignal = g_uartBuffer[pData] - '0';
        if(g_uartBuffer[pData + 1] != ',') bufSignal = bufSignal * 10 + g_uartBuffer[pData + 1] - '0';
        g_signalQuantity = bufSignal;
        sprintf(str,"GSM >> +Csq %d", g_signalQuantity);
        Dbg_Println(str);
        g_curTick = GetTickCount();
        g_state = 9;
        Serial_ClearRxBuffer(&g_serial);
      }
      else if(GetTickCount() - g_curTick > 3000)
      {
        g_curTick = GetTickCount();
        g_state = 9;
      }
      break;
    /* Trang thai 9: Cap nhan con tro doc va quay ve trang thai 0 */
    case 9:
        /* Cho trong vong 1s */
        if (GetTickCount() - g_curTick > 1000)
        {
            /* Neu co bao frame vua gui tren RAM */
            if (g_isFrameFromRam)
            {
                /* Update RAM den khung du lieu tiep theo */
                g_frameBufferOnRamGetPtr++;
                /* Neu vuot qua gioi han */
                if (g_frameBufferOnRamGetPtr >= GPRS_FRAME_ON_RAM_SIZE)
                {
                    /* Set quay lai dia chi dau */
                    g_frameBufferOnRamGetPtr = 0;
                }
            }
            /* Neu frame vua gui tren ROM */
            else
            {
                /* Neu frame vua gui khong phai la frame dong cua */
                if (g_gprsCurrentFrame.dataToSend[20] != 'd')
                {
                    /* Tang con tro doc len 1 khung du lieu (10 byte) */
                    g_eepromBufferGetPtr += 11;
                    /* Neu vuot qua gioi han */
                    if (g_eepromBufferGetPtr >= GPRS_EEP_BUFFER_END)
                    {
                        /* Set quay lai dia chi dau */
                        g_eepromBufferGetPtr = GPRS_EEP_BUFFER_START;
                    }
                    /* Luu vi tri con tro doc vao trong ROM */
                    SaveEepBufferGetPointer();
                }
                /* Neu frame vua gui la frame dong cua */
                else
                {
                    /* Xoa vi tri ghi do dai frame dong cua de bao frame da duoc gui va
                      khong con lai frame dong cua nao */
                    EE_WriteByte(GPRS_FRAME_EEP_ADDRESS, 0);
                }
            }
            /* Ghi du lieu vua gui vao file "log"*/
            Dbg_Println("Gsm >> Sending and response are successful");
            /* Dong file "log" -> ket thuc ghi */
            f_close(&g_logFile);
            /* Quay lai trang thai 0 va thuc hien chu trinh tiep theo */
            g_state = 0;
        }
        break;
    /* Trang thai 10: cho 30s khong lam gi ca */
    case 10:
        /* wait 30s */
        if (GetTickCount() - g_curTick > 30000)
        {
            Dbg_Println("Gsm >> 30 seconds passed");
            g_state = 0;
        }
        break;
    /* Trang thai 0xF0: Reset GSM (Tat nguon)*/
    case 0xF0:
        /* Reset the GSM module */
        Gsm_TurnOff();
        g_curTick = GetTickCount();
        g_state = 0xF1;
        break;
    /* Trang thai 0xF1: Reset GSM (Bat nguon)*/
    case 0xF1:
        if (GetTickCount() - g_curTick > 100)
        {
            Gsm_TurnOn();
            g_curTick = GetTickCount();
            g_state = 0xF2;
        }
        break;
    /* Trang thai 0xF2: Xoa RxBuffer va thuc hien lenh ATE0 */
    case 0xF2:
        if (GetTickCount() - g_curTick > 10000)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "ATE0\n\r", 6);
            Delay(500);
            DetectSimCard();
            g_curTick = GetTickCount();
            g_state = 0xF3;
        }
        break;
    /* Trang thai 0xF3: Cho lay co bao trang thai UART2 va thuc hien la
       tu dau (trang thai 0) */
    case 0xF3:
        if (GetTickCount() - g_curTick > 15000)
        {
            uint32_t tmp = LPUART_GetStatusFlags(LPUART2);
            g_state = 0;
        }
        break;
    /* Get name last update file in sim800 */
    case STATE_FTP_GET_LASTFILE:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            if(Serial_FindString(&g_serial, "VM") != NULL)
            {
                /* Clear name buffer */
                memset(g_vmNameFile, 0, 32);
                /* Get name file */
                int index = Serial_FindString(&g_serial, "VM") - (char*)g_uartBuffer;
                uint8_t countNameLen = 0;
                while(countNameLen < 32)
                {
                    if(g_uartBuffer[index] == 0x0D) break;
                    g_vmNameFile[countNameLen++] = g_uartBuffer[index++];
                }
                /* Delete last update file */
                memset(buf_str, 0, 64);
                Serial_ClearRxBuffer(&g_serial);
                uint8_t _length = sprintf(buf_str,"AT+FSDEL=C:\\User\\FTP\\%s\r\n", g_vmNameFile);
                Serial_Write(&g_serial, buf_str, _length);
                Dbg_Println(buf_str);
                g_curTick = GetTickCount();
                g_state = STATE_FTP_REMOVE_LASTFILE;
                Dbg_Println("Gsm >> FTP remove file in sim");
            }
            else
            {
                Dbg_Println("Gsm >> FTP not found file in sim");
                g_state = STATE_FTP_GPRS_CONFIG;
                g_curTick = GetTickCount();
                Dbg_Println("");
            }
        }
        else if (Serial_FindString(&g_serial, "ERROR") != NULL)
        {

              Dbg_Println("Gsm >> FTP not found file in sim");
              g_state = STATE_FTP_GPRS_CONFIG;
              g_curTick = GetTickCount();
        }
        break;
    /* Remove file last firmware */
    case STATE_FTP_REMOVE_LASTFILE:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            g_state = STATE_FTP_GPRS_CONFIG;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Remove last update success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }

        break;
    /* Connet FTP */
    case STATE_FTP_GPRS_CONFIG:
        /* Xoa toan bo bo dem nhan du lieu */
        Serial_ClearRxBuffer(&g_serial);
        /* Connect FTP */
        Serial_Write(&g_serial, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", 31);
        g_curTick = GetTickCount();
        g_state = STATE_FTP_GPRS_CONNECT;
        Dbg_Println("Gsm >> Start FTP...");
        break;

    case STATE_FTP_GPRS_CONNECT:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+SAPBR=1,1\r\n", 14);
            g_state = STATE_FTP_PROFILE;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Gprs config success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_PROFILE:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+FTPCID=1\r\n", 13);
            g_state = STATE_FTP_SERVER_SET;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Gprs connect");
        }
        else if(Serial_Available(&g_serial) > 2)
        {
          Serial_ClearRxBuffer(&g_serial);
          Serial_Write(&g_serial, "AT+FTPCID=1\r\n", 13);
          g_state = STATE_FTP_SERVER_SET;
          g_curTick = GetTickCount();
          Dbg_Println("Gsm >> Gprs connect");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_SERVER_SET:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+FTPSERV=\"203.171.20.53\"\r\n", 28);
            g_state = STATE_FTP_PORT_SET;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> FTP profile ...");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_PORT_SET:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+FTPPORT=\"21\"\r\n", 17);
            g_state = STATE_FTP_USER_SET;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Server set infor success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_curTick = GetTickCount();
            g_ftpErrorCount++;
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_USER_SET:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+FTPUN=\"vms\"\r\n", 16);
            g_state = STATE_FTP_PW_SET;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Server set port success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_PW_SET:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+FTPPW=\"Vending@123!\"\r\n", 25);
            g_state = STATE_FTP_NAME_FILE;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Set user name success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_NAME_FILE:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            /* lay len file */
            for(uint8_t index = 0; index < 64; index ++)
            {
              buf_str[index] = 0;
            }
            ftp_temp = 0;
             ftp_temp = sprintf(buf_str, "AT+FTPGETNAME=");
            buf_str[ftp_temp++] = '"';
            for(uint8_t count = 0; count < g_lenUpdateFileName; count++)
            {
            buf_str[ftp_temp++] = g_updateFileName[count];
            }
            buf_str[ftp_temp++] = '"';
            buf_str[ftp_temp++] = '\r';
            buf_str[ftp_temp++] = '\n';

            /* send name file */
            Serial_Write(&g_serial, buf_str, ftp_temp);
            g_state = STATE_FTP_PATH_FILE;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Set user PW success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_PATH_FILE:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "AT+FTPGETPATH=\"/\"\r\n", 19);
            g_state = STATE_FTP_GETFILE;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Set name file success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_GETFILE:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            /* get fime name */
        for(uint8_t index = 0; index < 64; index ++)
        {
          buf_str[index] = 0;
        }
            ftp_temp = 0;
        ftp_temp = sprintf(buf_str, "AT+FTPGETTOFS=0,");
            buf_str[ftp_temp++] = '"';
            for(uint8_t count = 0; count < g_lenUpdateFileName; count++)
            {
            buf_str[ftp_temp++] = g_updateFileName[count];
            }
            buf_str[ftp_temp++] = '"';
            buf_str[ftp_temp++] = '\r';
            buf_str[ftp_temp++] = '\n';

            Serial_Write(&g_serial, buf_str, ftp_temp);
            g_state = STATE_FTP_CHECK_GET;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Set path success");
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_CHECK_GET:
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Neu co phan hoi ve thanh cong */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            Serial_ClearRxBuffer(&g_serial);
            g_state = STATE_FTP_CHECK_GET_SUCCESS;
            g_curTick = GetTickCount();
            Dbg_Println("Gsm >> Start get file");
            ftp_check_tick =  GetTickCount();
        }
        else if (GetTickCount() - g_curTick > 3000)
        {
            g_ftpErrorCount++;
            g_curTick = GetTickCount();
            if (g_ftpErrorCount > 5)
            {
                g_state = STATE_FTP_RETURN;
            }
        }
        break;
    case STATE_FTP_CHECK_GET_SUCCESS:
        /* neu qua thoi gian dowload thi thoat khoi vong doi*/
        if (GetTickCount() - g_curTick > 90000)
        {
            g_state = STATE_FTP_RETURN;
            g_ftpErrorCount = 10;
            Dbg_Println("Gsm >> Download file update: FALSE");
        }
        else
        {
            if(GetTickCount() > ftp_check_tick)
            {
                if (Serial_FindString(&g_serial, "+FTPGETTOFS: 0") != NULL)
                {
                    g_state = STATE_FTP_RETURN;
                    Dbg_Println("Gsm >> Download file update: DONE");
                }
                ftp_check_tick = GetTickCount() + 500;
            }
        }
        break;
    case STATE_FTP_RETURN:
        if (g_ftpErrorCount > 5) /* NEU CAP NHAT KHONG THANH CONG */
        {
            g_state = g_savestate;
        }
        else                     /* NEU CAP NHAT THANH CONG */
        {
            /* save file update information to eeprom */
            WriteVerifyDataFlag(FILE_UPDATE_CRC_ERROR_ADDRESS, 0xAA);
            WriteVerifyDataFlag(UPDATE_AUTO_FLAG_ADDRESS, 0x55);
            WriteVerifyDataFlag(LENGTH_FILE_UPDATE_NAME_ADDRESS, g_lenUpdateFileName);
            WriteVerifyData(FILE_UPDATE_NAME_ADDRESS, g_updateFileName, g_lenUpdateFileName);
            updateFirmware = READY_UPDATE;
            g_state = g_savestate;
            ftp_count_down++;
        }
        break;
    }
}

extern bool Door_IsOpen();
/**
 * @brief   Ham tao frame gui du lieu theo chu ki luu tru vao RAM (Ban tin theo chu ki)
 * @param   NONE
 * @retval  NONE
 */
static void Gsm_SendDataPeriod()
{
    int hour = 0, minute = 0, second = 0, date = 0, month = 0, year = 18;
    /* Lay ID cua may (6 so), la so tu nhien */
    Perh_GetVendId(&g_FrameCycle[8]);

    for (int i = 8; i < 14; i++)
    {
        /* Lay tung so trong ID + 0x30 -> Chuyen thanh ki tu, luu tru ki tu vao byte dia chi 8 -> 13*/
        g_FrameCycle[i] += '0';
    }
    /* Cap nhan trang thai dong mo cua */
    if (Door_IsOpen())
    {
        /* Cua dang mo */
        g_FrameCycle[22] = '1';
    }
    else
    {
        /* Cua dang dong */
        g_FrameCycle[22] = '0';
    }
    /* Cap nhat gia tri do am */
    g_FrameCycle[23] = g_gsmHumidityValueMsg;
    /* Cap nhat trang thai dau doc tien */
    if (NV11_GetLatestEvent() == NV11_ERROR)
    {
        /* Neu dau doc tien dang bi loi */
        g_FrameCycle[24] = '0';
        for (int i = 0; i < 9; i++)
        {
            /* Xoa toan bo gia tri trong byte dia chi 25 -> 34 */
            g_FrameCycle[25 + i] = 0;   /* So to tien/ kenh -> xoa ve 0 */
        }
    }
    else
    {
        /* Neu dau doc tien khong bi loi */
        g_FrameCycle[24] = '1';
        for (int i = 0; i < 9; i++)
        {
            g_FrameCycle[25 + i] = 0;   /* So to tien/ kenh -> xoa ve 0 */
        }
        /* Neu dau doc la NV11 -> Chi co 1 menh gia tra lai */
    /* byte |  25 |  26  |  27  |   28  |   29  |  30   |   31  |   32   |    33  | */
    /* Note | 1000| 2000 | 5000 | 10000 | 20000 | 50000 | 100000| 200000 | 500000 |*/
        if (NV11_GetUnitType() == NV_TYPE_NV11)
        {
            /* Tinh so to tien tren FloatNote (tra lai) luu vao byte dia chi 27 */
        uint8_t noteNumber = NV11_GetAvailableChange() / NV11_GetDenominationForChange();
        switch(NV11_GetDenominationForChange())
        {
            case 1000:
          g_FrameCycle[25] = noteNumber;
          break;
        case 2000:
          g_FrameCycle[26] = noteNumber;
          break;
        case 5000:
          g_FrameCycle[27] = noteNumber;
          break;
        case 10000:
          g_FrameCycle[28] = noteNumber;
          break;
        case 20000:
          g_FrameCycle[29] = noteNumber;
          break;
        case 50000:
          g_FrameCycle[30] = noteNumber;
          break;
        case 100000:
          g_FrameCycle[31] = noteNumber;
          break;
        case 200000:
          g_FrameCycle[32] = noteNumber;
          break;
        case 500000:
          g_FrameCycle[33] = noteNumber;
          break;
        }

        }
        /* Neu dau doc la NV200 -> Co nhieu menh gia tra lai */
        else if (NV11_GetUnitType() == NV_TYPE_NV200)
        {
            /* Tinh so to tien cua moi menh gia tren FloatNote (tra lai) luu vao dia chi tu 25 ~ 33 */
            for (int channelIndex = 0; channelIndex < 9; channelIndex++)
            {
                g_FrameCycle[25 + channelIndex] = NV11_GetStoredNoteByChannel(channelIndex);   /* The number of change notes */
            }
        }
    }
    /* So hang hoa hien tai tren tung khay */
    for (int i = 34; i < 94; i++)
    {
        /* Lay so luong hang tren tung khay luu vao byte dia chi tu 34 ~ 93 (60 khay)*/
        if(g_is_half_stock) {
            g_FrameCycle[i] = 0x00;//Perh_GetItemNumber(i - 33);
        } else {
            g_FrameCycle[i] = 0xFF;
        }
//        g_FrameCycle[i] = Perh_GetItemNumber(i - 33);
    }
    /* Update thoi gian hien tai tu RTC */
    Rtc_GetCurrentTime(&hour, &minute, &second);
    Rtc_GetCurrentDate(&date, &month, &year);
    g_FrameCycle[94] = year;
    g_FrameCycle[95] = month;
    g_FrameCycle[96] = date;
    g_FrameCycle[97] = hour;
    g_FrameCycle[98] = minute;
    g_FrameCycle[99] = second;
    /* version */
    g_FrameCycle[100]= g_FirmwareVersion[0];
    g_FrameCycle[101]= g_FirmwareVersion[1];
    g_FrameCycle[102]= g_FirmwareVersion[2];
    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameCycle[3], 101);
    g_FrameCycle[104] = (uint8_t)(crc >> 8);
    g_FrameCycle[105] = (uint8_t)crc;
    /* Luu tru du lieu vao RAM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameCycle, 109);
}

/**
 * @brief   Ham tao frame bao trang thai dau doc NV11 va luu tru du lieu EEPROM
 * @param   NONE
 * @retval  NONE
 */
static void SendNV11ErrorFrame()
{
    static bool nv11IsError = false;
    static uint32_t curTick = 0;
    /* Neu thoi gian xay ra loi qua 1s */
    if (GetTickCount() - curTick > 1000)
    {
        curTick = GetTickCount();
        /* Neu bien bao loi chua nhan dang NV11 dang loi va su kien gan nhat bao NV11 loi */
        if (nv11IsError == false && NV11_GetLatestEvent() == NV11_ERROR)
        {
            nv11IsError = true;                 /* Thiet lap bien nhan dang NV11 loi */

            /* Luu tru ID cua thiet bi loi (NV11 = 0x01) vao byte dia chi 22 */
            g_FrameDeviceError[22] = 0x61;
            /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
            g_FrameDeviceError[23] = 0x30;

            /* Tinh CRC */
            uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
            g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
            g_FrameDeviceError[26] = (uint8_t)crc;

            /* Luu tru du lieu vao ROM */
            Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
        }
        /* Neu bien bao loi nhan dang NV11 dang loi, va su kien gan nhan NV11 khong loi */
        else if (nv11IsError == true && NV11_GetLatestEvent() != NV11_ERROR)
        {
            nv11IsError = false;                /* Huy thiet lap bien bao loi (xoa loi) */

            /* Luu tru ID cua thiet bi loi (NV11 = 0x61) vao byte dia chi 22 */
            g_FrameDeviceError[22] = 0x61;
            /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
            g_FrameDeviceError[23] = 0x31;

            /* Tinh CRC */
            uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
            g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
            g_FrameDeviceError[26] = (uint8_t)crc;

            /* Luu tru du lieu vao ROM */
            Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
        }

        if (NV11_GetLatestEvent() == NV11_ERROR)
        {
            nv11IsError = true;
        }
        else
        {
            nv11IsError = false;
        }
    }
}
/**
 * @brief   Ham tao frame ban tin cap nhat thoi gian thuc (0x70)
 * @param   NONE
 * @retval  NONE
 */
static void SendTimeRequestFrame()
{
    static uint32_t oldTick = 0;
    int hour = 0, minute = 0, second = 0;
    int date = 0, month = 0, year = 0;
    bool restime = false;
    bool resdate = false;
    uint8_t failCount = 0;
    /* Kiem tra RTC sau moi 50s */
    if (GetTickCount() - oldTick > 50000)
    {
        oldTick = GetTickCount();
        while (1)
        {
            /* Doc du lieu gio, phut, giay tu RTC */
            restime = Rtc_GetCurrentTime(&hour, &minute, &second);
            resdate = Rtc_GetCurrentDate(&date, &month, &year);
            /* Neu doc RTC ok */
            if ((restime == true)&&(resdate == true))
            {
                break;  /* Thoat khoi vong lap */
            }
            /* Neu doc RTC loi */
            else
            {
                /* Thu trong vong 5 lan */
                failCount++;
                if (failCount >= 5)
                {
                    return;     /* Neu qua 5 lan khong doc duoc -> thoat vong lap */
                }
            }
        }
        /* Neu thoi gian luc nay la 0h:0m:ns (12 gio dem) hoac nam khong hop le */
        if((hour == 0 && minute == 0)||(year < 19))
        {
            CreatTimeRequestFrame();
        }
        /* Luu y, Frame khong mang theo du lieu, CMD = 0x70 */
    }
}
void CreatTimeRequestFrame (void)
{
    Perh_GetVendId(&g_FrameTimeRequest[8]);
    for (int i = 8; i < 14; i++)
    {
        /* Lay tung so trong ID + 0x30 -> Chuyen thanh ki tu, luu tru ki tu vao byte dia chi 8 -> 13*/
        g_FrameTimeRequest[i] += '0';
    }
    uint16_t crc = CRC_XModem(&g_FrameTimeRequest[3], 21);
    g_FrameTimeRequest[24] = (uint8_t)(crc >> 8);
    g_FrameTimeRequest[25] = (uint8_t)crc;
    /* Luu du lieu vao RAM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameTimeRequest, 29);
}
/**
 * @brief   Ham tao frame bao loi RTC va luu tru du lieu EEPROM
 * @param   NONE
 * @retval  NONE
 */
static void SendRtcErrorFrame()
{
    /* Luu tru ID cua thiet bi loi (RTC = 0x72) vao byte dia chi 22 */
    g_FrameDeviceError[22] = 0x72;
    /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x30;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}

/**
 * @brief   Ham tao frame bao da sua RTC va luu tru du lieu EEPROM
 * @param   NONE
 * @retval  NONE
 */
void SendRtcResumeFrame()
{
    /* Luu tru ID cua thiet bi loi (RTC = 0x72) vao byte dia chi 22 */
    g_FrameDeviceError[22] = 0x72;
    /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x31;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}
/**
 * @brief   Ham tao frame bao loi EEPROM va luu tru du lieu RAM
 * @param   NONE
 * @retval  NONE
 */
static void SendEepromErrorFrame()
{
    /* Luu tru ID cua thiet bi loi (EEPROM = 0x70) vao byte dia chi 22 */
    g_FrameDeviceError[22] = 0x70;
    /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x30;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao RAM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}
/**
 * @brief   Ham tao frame bao sua xong EEPROM va luu tru du lieu RAM
 * @param   NONE
 * @retval  NONE
 */
void SendEepromResumeFrame()
{
    /* Luu tru ID cua thiet bi loi (EEPROM = 0x70) vao byte dia chi 22 */
    g_FrameDeviceError[22] = 0x70;
    /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x31;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao RAM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}
/**
 * @brief   Ham tao frame bao loi the nho va luu tru du lieu ROM
 * @param   NONE
 * @retval  NONE
 */
void SendSDCardErrorFrame()
{
    /* Luu tru ID cua thiet bi loi (The nho = 0x69) vao byte dia chi 22 */
    g_FrameDeviceError[22] = 0x69;
    /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x30;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}
/**
 * @brief   Ham tao frame bao sua xong the nho va luu tru du lieu ROM
 * @param   NONE
 * @retval  NONE
 */
void SendSDCardResumeFrame()
{
    /* Luu tru ID cua thiet bi loi (The nho = 0x69) vao byte dia chi 22 */
    g_FrameDeviceError[22] = 0x69;
    /* Luu tru trang thai cua thiet bi: 0x30 = Dang loi, 0x31 = Da sua loi */
    g_FrameDeviceError[23] = 0x31;

    /* Tinh CRC */
    uint16_t crc = CRC_XModem(&g_FrameDeviceError[3], 22);
    g_FrameDeviceError[25] = (uint8_t)(crc >> 8);
    g_FrameDeviceError[26] = (uint8_t)crc;

    /* Luu du lieu vao ROM */
    Gsm_SendData(GPRS_MAIN_SERVER, g_FrameDeviceError, 30);
}
/**
 * @brief   Ham khoi tao mot cuoc goi
 * @param   NONE
 * @retval  NONE
 */
void Gsm_MakeCall()
{
    if (!g_isCalling)
    {
        g_callRequest = 1;  /* Yeu cau thuc hien cuoc goi */
        g_isCalling = true;
    }
}

/**
 * @brief   Ham yeu cau gach may
 * @param   NONE
 * @retval  NONE
 */
void Gsm_HangCall()
{
    if (g_isCalling)
    {
        g_callRequest = 2;  /* Yeu cau gac may */
    }
}
/**
 * @brief   Ham tra trang thai dang thuc hien cuoc goi
 * @param   NONE
 * @retval  NONE
 */
bool Gsm_IsCalling()
{
    return g_isCalling;
}
/**
 * @brief   Ham lay thong tin IP & Port server tu cai dat nguoi dung
 * @param   uint8_t ip1 -> IP server number 1
 * @param   uint8_t ip2 -> IP server number 2
 * @param   uint8_t ip3 -> IP server number 3
 * @param   uint8_t ip4 -> IP server number 4
 * @param   uint32_t port -> Port Server
 * @retval  NONE
 */
void Gsm_SetServerInfo(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4, uint32_t port)
{
    sprintf(g_serverIP, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    sprintf(g_serverPort, "%d", port);
}
/**
 * @brief   Ham xu ly cuoc goi toi tong dai
 * @param   NONE
 * @retval  NONE
 */
void Gsm_CallProcess()
{
    static uint8_t dotCount = 0;
    static uint32_t curTick = 0;
    static uint8_t fail = 0;
    char phoneNumber[16];

    switch (g_callState)
    {
    /* Trang thai 0: Bat dau thuc hien cuoc goi */
    case 0:
        /* Dang co 1 yeu cau cuoc goi can thuc hien */
        if (g_callRequest == 1)
        {
            g_callRequest = 0;  /* Xoa yeu cau*/
            /* Neu lay so dien thoai goi trong ROM bi loi */
            if (Perh_GetOperatorNumber(phoneNumber) == false)
            {
                /* Hien thi ra man hinh LCD "CALLING ERROR"*/
                GLcd_ClearScreen(BLACK);
                int x = (GLCD_WIDTH - GLcd_MeasureStringUni(Lang_GetText(LANG_CALLING_ERROR))) / 2;
                GLcd_DrawStringUni(Lang_GetText(LANG_CALLING_ERROR), x, 25, WHITE);
                GLcd_Flush();
                Delay(1500);
                /* Xoa co bao thuc hien cuoc goi */
                g_isCalling = false;
                /* Dat trang thai xu ly cuoc goi ve 0 */
                g_callState = 0;
                break;  /* Thoat khoi case */
            }
            /* Xoa toan bo Rx buffer */
            Serial_ClearRxBuffer(&g_serial);
            /* Gui lenh toi GSM thuc hien goi den 1 so */
            Serial_Write(&g_serial, "ATD", 3);
            /* Gui so dien thoai goi den va do dai */
            Serial_Write(&g_serial, phoneNumber, strlen(phoneNumber));
            Serial_Write(&g_serial, ";\n\r", 3);
            /* Chuyen sang trang thai 2: Cho phan hoi tu GSM */
            g_callState = 1;
            curTick = GetTickCount();
        }
        break;
    /* Trang thai 2: Cho phan hoi tu GSM */
    case 1:
        /* Neu cho nhan duoc bat ki du lieu nao -> Cho nha du lieu */
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Cho den khi nhan duoc chuoi ki tu gui ve "OK", hoac qua thoi gian cho */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            /* Hien thi len man hinh LCD: "CALLING"*/
            GLcd_ClearScreen(BLACK);
            GLcd_DrawStringUni(Lang_GetText(LANG_CALLING), 30, 25, WHITE);
            GLcd_Flush();
            curTick = GetTickCount();
            /* Chuyen sang trang thai 2: Xu ly cuoc goi */
            g_callState = 2;
        }
        /* Neu qua 0.5s ma khong nhan duoc bat ki phan hoi nao tu GSM */
        else if (GetTickCount() - curTick > 500)
        {
            g_callRequest = 1;  /* Dat lai yeu cau cuoc goi */
            g_callState = 0;    /* Chuyen ve trang thai 0, thuc hien lai cuoc goi */
            /* Tinh so lan thuc hien cuoc goi loi */
            fail++;
            /* Neu qua 5 lan */
            if (fail > 5)
            {
                fail = 0;
                /* Hien thi ra man hinh LCD "CALLING ERROR"*/
                GLcd_ClearScreen(BLACK);
                int x = (GLCD_WIDTH - GLcd_MeasureStringUni(Lang_GetText(LANG_CALLING_ERROR))) / 2;
                GLcd_DrawStringUni(Lang_GetText(LANG_CALLING_ERROR), x, 25, WHITE);
                GLcd_Flush();
                Delay(1500);
                /* Xoa co bao thuc hien cuoc goi */
                g_isCalling = false;
                /* Huy yeu cau thuc hien cuoc goi */
                g_callRequest = 0;
                /* Chuyen ve trang thai 0, cho yeu cau */
                g_callState = 0;
                break;
            }
        }
        break;
    /* Trang thai 2: Xu ly cuoc goi */
    case 2:
        /* Neu nhan duoc yeu cau gach may */
        if (g_callRequest == 2)
        {
            /* Xoa yeu cau thuc hien cuoc goi */
            g_callRequest = 0;
            /* Gui lenh AT toi GSM huy ket noi */
            Serial_ClearRxBuffer(&g_serial);
            Serial_Write(&g_serial, "ATH\n\r", 5);
            /* Chuyen sang trang thai 3: */
            g_callState = 3;
            curTick = GetTickCount();
        }
        /* Neu GSM phan hoi ve chuoi ki tu "NO ANSWER" = Khong tra loi */
        else if (Serial_FindString(&g_serial, "NO ANSWER") != NULL)
        {
            /* Hien thi ra man hinh LCD "NO ANSWER"*/
            GLcd_ClearScreen(BLACK);
            int x = (GLCD_WIDTH - GLcd_MeasureStringUni(Lang_GetText(LANG_NO_ANSWER))) / 2;
            GLcd_DrawStringUni(Lang_GetText(LANG_NO_ANSWER), x, 25, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Xoa co bao thuc hien cuoc goi */
            g_isCalling = false;
            /* Chuyen ve trang thai 0 cho yeu cau cuoc goi */
            g_callState = 0;
        }
         /* Neu GSM phan hoi ve chuoi ki tu "BUSY" = So may ban */
        else if (Serial_FindString(&g_serial, "BUSY") != NULL)
        {
            /* Hien thi ra man hinh LCD "NUMBER BUSY"*/
            GLcd_ClearScreen(BLACK);
            int x = (GLCD_WIDTH - GLcd_MeasureStringUni(Lang_GetText(LANG_NUMBER_BUSY))) / 2;
            GLcd_DrawStringUni(Lang_GetText(LANG_NUMBER_BUSY), x, 25, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Xoa co bao thuc hien cuoc goi */
            g_isCalling = false;
            /* Chuyen ve trang thai 0 cho yeu cau cuoc goi */
            g_callState = 0;
        }
        /* Neu GSM phan hoi ve chuoi ki tu "NO CARRIER" = Mat tin hieu */
        else if (Serial_FindString(&g_serial, "NO CARRIER") != NULL)
        {
            /* Hien thi ra man hinh LCD "SIGNAL LOSST"*/
            GLcd_ClearScreen(BLACK);
            int x = (GLCD_WIDTH - GLcd_MeasureStringUni(Lang_GetText(LANG_SIGNAL_LOST))) / 2;
            GLcd_DrawStringUni(Lang_GetText(LANG_SIGNAL_LOST), x, 25, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Xoa co bao thuc hien cuoc goi */
            g_isCalling = false;
            /* Chuyen ve trang thai 0 cho yeu cau cuoc goi */
            g_callState = 0;
        }
        /* Neu GSM phan hoi ve chuoi ki tu "+CME ERROR": Xay ra loi cuoc goi */
        else if (Serial_FindString(&g_serial, "+CME ERROR:") != NULL)
        {
            /* Hien thi ra man hinh LCD "CALLING ERROR" */
            GLcd_ClearScreen(BLACK);
            int x = (GLCD_WIDTH - GLcd_MeasureStringUni(Lang_GetText(LANG_CALLING_ERROR))) / 2;
            GLcd_DrawStringUni(Lang_GetText(LANG_CALLING_ERROR), x, 25, WHITE);
            GLcd_Flush();
            Delay(1500);
            /* Xoa co bao thuc hien cuoc goi */
            g_isCalling = false;
            /* Chuyen ve trang thai 0 cho yeu cau cuoc goi */
            g_callState = 0;
        }
        /* Neu qua toi gian 0.5s khong nhan duoc phan hoi*/
        else if (GetTickCount() - curTick > 500)
        {
            curTick = GetTickCount();
            /* Hien thi ra man hinh LCD "CALLING ..." */
            GLcd_ClearScreen(BLACK);
            GLcd_SetFont(&font6x8);
            int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_CALLING));
            GLcd_DrawStringUni(Lang_GetText(LANG_CALLING), 30, 25, WHITE);

            if (dotCount == 0)
            {
                dotCount = 1;
            }
            else if (dotCount == 1)
            {
                GLcd_DrawString(".", sizeInPixel + 31, 31, WHITE);
                dotCount = 2;
            }
            else if (dotCount == 2)
            {
                GLcd_DrawString("..", sizeInPixel + 31, 31, WHITE);
                dotCount = 3;
            }
            else
            {
                GLcd_DrawString("...", sizeInPixel + 31, 31, WHITE);
                dotCount = 0;
            }
            GLcd_Flush();
        }
        break;
    /* Trang thai 3: Cho GSM phan hoi lenh gac may */
    case 3:
        /* Neu chua nhan duoc bat ki du lieu nao -> cho nhan du lieu */
        if (Serial_Available(&g_serial) && Serial_PeekByte(&g_serial) == 0)
        {
            Serial_ReadByte(&g_serial);
        }
        /* Cho den khi nhan duoc chuoi "OK" hoac qua thoi gian */
        if (Serial_FindString(&g_serial, "OK") != NULL)
        {
            /* Xoa co bao thuc hien cuoc goi */
            g_isCalling = false;
            /* Chuyen ve trang thai 0 cho yeu cau cuoc goi moi */
            g_callState = 0;
        }
        /* Neu qua 0.5s khong nhan duoc bat ki phan hoi nao */
        else if (GetTickCount() - curTick > 500)
        {
            fail++;
            /* Tinh so lan loi, neu qua 2 lan */
            if (fail > 2)
            {
                fail = 0;
                /* Xoa co bao thuc hien cuoc goi */
                g_isCalling = false;
                /* Xoa yeu cau thuc hien cuoc goi */
                g_callRequest = 0;
                /* Chuyen ve trang thai 0 cho yeu cau cuoc goi moi */
                g_callState = 0;
            }
            /* Neu chua qua 2 lan, quay lai trang thai 2 va yeu cau gac may*/
            else
            {
                g_callRequest = 2;
                g_callState = 2;
            }
        }
        break;
    }
}


static char* Gsm_FindString(uint8_t* data, uint8_t lengofdata, char* str, uint8_t lengofstr)
{
  char* posBuf = NULL;
  bool findFlag = false;
  for(uint32_t index = 0; index < lengofdata; index++)
  {
    if(data[index] == str[0])
    {
      findFlag = true;
      posBuf = (char*)(data + index);
      for(uint8_t count = 0; count < lengofstr; count++)
      {
    if(data[index + count] != str[count])
    {
      findFlag = false;
    }
      }

      if(findFlag) return posBuf;
    }
  }
  return 0;
}

static void Gsm_check_response(response_t* response)
{
  bool id_flag = true;
  uint32_t oldtick = GetTickCount();
  char buf_data[100] = {0};
  uint8_t machine[6] = {0};
  union{
    uint16_t crc_16;
    uint8_t crc_8[2];
  }crc;
  char* foundStartChar;
  char* foundEndChar;
  int foundPosStart;
  int foundPosEnd = 0;
  /* lay id may trong eeprom */
  Perh_GetVendId(machine);
  /* lay toan bo du lieu phan hoi tu server */
  uint16_t Datalength = Serial_Available(&g_serial);
  for (uint8_t count = 0; count < Datalength; count++)
  {
    buf_data[count] = Serial_ReadByte(&g_serial);
  }

  /* phan tich du lieu co trong buffer */
  while(1)
  {
    /* tim kiem ki tu bat dau frame */
    foundStartChar = Gsm_FindString(&buf_data[foundPosEnd], 100 - foundPosEnd, "###", 3);
    if(foundStartChar == NULL) break;
    foundPosStart = foundStartChar - buf_data;
    /* tim kiem ki tu ket thuc frame */
    foundEndChar = Gsm_FindString(&buf_data[foundPosStart], 100 - foundPosStart, "&&&", 3);
    foundPosEnd = foundEndChar - buf_data;

    /* Log lai du lieu vua nhan duoc */
    memset(FrameToLog,0,600);
    Dbg_Print("Gsm >> DataReceive: ");
    char str[5];
    for (uint8_t i = 0; i < foundPosEnd - foundPosStart + 3; i++)  /* cong 3 byte ket thuc */
    {
        sprintf(str, "%02x ", buf_data[i + foundPosStart]);
        FrameToLog[0 + i*2] = str[0];
        FrameToLog[1 + i*2] = str[1];
    }
    Dbg_Println(FrameToLog);

    /* kiem tra xem dung id may khong */
    id_flag = true;
    for (int i = 0; i < 6; ++i)
    {
      if (machine[i] != buf_data[i + foundPosStart + 8] - 0x30)
      {
        id_flag = false;
      }
    }

    if (id_flag)
    {
      /* kiem tra loai ban tin tra ve */
      if(buf_data[foundPosStart + 20] == 'v' || buf_data[foundPosStart + 20] == 'w')
      {
        crc.crc_8[1] = buf_data[foundPosStart + 29];
        crc.crc_8[0] = buf_data[foundPosStart + 30];
        if(crc.crc_16 == CRC_XModem(&buf_data[foundPosStart+3], 26))
        {
          if(buf_data[foundPosStart + 20] == 'v')response->success_flag = RESPONSE_SUCCESS;
          else response->success_flag = RESPONSE_ERROR;
          response->time.year    = buf_data[foundPosStart + 3 + 19];
          response->time.month   = buf_data[foundPosStart + 3 + 20];
          response->time.date    = buf_data[foundPosStart + 3 + 21];
          response->time.hours   = buf_data[foundPosStart + 3 + 22];
          response->time.minute  = buf_data[foundPosStart + 3 + 23];
          response->time.seconds = buf_data[foundPosStart + 3 + 24];
        }
        else
        {
          Dbg_Println("error crc in response \r\n");
        }
        continue;
      }
      else if (buf_data[foundPosStart + 20] == 'u') /* kiem tra ban tin nhan ve la update */
      {
        crc.crc_8[1] = buf_data[foundPosStart + 26];
        crc.crc_8[0] = buf_data[foundPosStart + 27];
        if(crc.crc_16 == CRC_XModem(&buf_data[foundPosStart+3], 23))
        {
          response->update_flag = 0x01;
          response->version[0] = buf_data[foundPosStart + 3 + 19];
          response->version[1] = buf_data[foundPosStart + 3 + 20];
          response->version[2] = buf_data[foundPosStart + 3 + 21];
        }
      }
    }
  }
}



static void WriteVerifyDataFlag(uint32_t address, uint8_t data)
{
  uint8_t failcount = 0;
  uint8_t countSuccess = 0;
  EE_Write(address, &data, 1);
  /* verify data */
  while(1)
  {
    uint8_t tmp = 0;
    /* doc lai gia tri co */
    EE_Read(address, &tmp, 1);
    if(tmp == data)
    {
      countSuccess++;
      if(countSuccess == 10) break;
      break;
    }
    else
    {
      EE_Write(address, &data, 1);
      failcount++;
      if(failcount > 10)
      {
        break;
      }
    }
  }
}

static void ReadVerifyDataFlag(uint32_t address, uint8_t* data)
{
  /* Read data */
  uint8_t bufdata = 0;
  uint8_t countSuccess = 0;
  uint8_t countRead = 0;
  EE_Read(address, &bufdata, 1);
  while(1)
  {
    uint8_t tmp = 0;
    EE_Read(address, &tmp, 1);
    if(tmp == bufdata)
    {
      countSuccess++;
      if(countSuccess == 10) break;
    }
    else
    {
      bufdata = tmp;
      countSuccess = 0;
    }
    countRead++;
    if(countRead == 30) break;
  }
  *data = bufdata;
}

static void WriteVerifyData(uint32_t address, uint8_t* data, int length)
{
  int countErrorWrite = 0;
  uint8_t tmpArray[64] = {0};
  /* Write data */
  EE_Write(address, data, length);
  /* */
  while(1)
  {
    /* doc lai ten version moi luu tru */
    EE_Read(address, tmpArray, length);
    /* kiem tra lai du lieu vua ghi */
    for(int count = 0; count < length; count++)
    {
      if(data[count] !=  tmpArray[count])
      {
        Delay(2);
        /* ghi lai du lieu */
        EE_Write(address, data, length);
        count = -1;
        countErrorWrite++;
        if(countErrorWrite > length)
        {
          Dbg_Println("Gsm >> Write Verify Data Error");
          return;
        }
      }
    }
    break;
  }
}

static void ReadVerifyData(uint32_t address, uint8_t* data, int length)
{
  uint8_t bufData[32] = {0};
  uint8_t count = 0;
  uint8_t countRead = 0;
  uint8_t countSuccess = 0;
  /* Doc du lieu lan dau */
  EE_Read(address, data, length);
  while(1)
  {
    /* Doc lai du lieu */
    EE_Read(address, bufData, length);
    for(count = 0; count < length; count++)
    {
      if(bufData[count] != data[count])
      {
        break;
      }
    }
    if(count == length) countSuccess++;
    if(countSuccess == 10) break;
    /* kiem tra lai du lieu */
    countRead++;
    if(countRead == 30) break;
  }
  /* copy du lieu tu buffer data sang */
  memcpy(data, bufData, length);
}
