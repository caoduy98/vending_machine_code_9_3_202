/** @file    peripheral.c
  * @author
  * @version
  * @brief   Cung cap cac ham o mid layer dieu khien ngoai vi tren bo mach
  */
#include "peripheral.h"
#include "eeprom.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "nv11.h"
#include "gsm.h"
#include "ftp_define.h"
#include "watchdog.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_PERIPHERAL
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif

#define LED_PIN             PA8
#define DROP_SENSOR_PIN     PD11

extern void Door_Init();
extern void FactoryReset();
extern bool Door_IsOpen();

static void RestoreItemsFromEeprom();

static bool g_dropSensorFlag = false;
extern uint8_t g_humidityset;
extern uint32_t g_n1_set;
extern uint32_t g_n2_set;
extern uint32_t g_n3_set;
static item_t g_itemList[RESOURCE_ITEM_MAX];

static bool g_heaterIsOn =  false;
static bool g_dropSensorIsOn = false;
static bool g_audioIsOn = false;
static bool g_refundIsOn = false;
static uint8_t g_acceptableNoteLow = 0;
static uint8_t g_acceptableNoteHigh = 0;
static uint8_t g_machineID[6] = { 0 };

static uint8_t g_callVolume = 10;
static uint8_t g_mediaVolume = 10;
static uint8_t g_microphoneLevel = 100;

static uint8_t g_slotColumn = 6;
static uint8_t g_slotRow = 6;

uint8_t g_FirmwareVersion[3] = {0};
/**
 * @brief  Ham xu ly ngat nhan cam bien roi
 *
 * @param  NONE
 * @retval NONE
 */
volatile uint32_t tick_hight_drop_sensor = 0;
volatile uint32_t count_change_state_drop_sensor = 0;

void PORTBCD_IRQHandler()
{
    /* Xoa co bao ngat tren chan  GPIO */
    GPIO_PortClearInterruptFlags(GPIOD, 1U << 11);
    /* Xay ra ngat GPIO, kiem tra chan DROP_SENSOR o muc thap*/
    uint8_t pin_status = Pin_Read(DROP_SENSOR_PIN);
    if (pin_status == 0)
    {
        /* Set co bao cam bien roi (hang da roi xuong ngan lay) */
        g_dropSensorFlag = true;
    }
    else if (pin_status == 1)
    {
      if(count_change_state_drop_sensor == 0)
      {
        tick_hight_drop_sensor = GetTickCount();
      }
      count_change_state_drop_sensor++;
    }
}

void Perh_CheckDropSensorProcess(void)
{
    if(GetTickCount() - tick_hight_drop_sensor > 5000 && count_change_state_drop_sensor != 0)
    {
      sprintf(bufferDebug, "Drop Sensor >> Number count %d", count_change_state_drop_sensor);
      Dbg_Println(bufferDebug);

      /* Hien thi thong bao may bi loi cam bien roi neu trong 5s co lon hon 5 lan thay vat xuat hien */
      if(count_change_state_drop_sensor > 10)
      {
        /* Khoa ban hang neu cua dang dong */
        if (!Door_IsOpen())
        {
            Dbg_Println("Drop Sensor >> Khoa may ban hang");
            Perh_SetErrorDropSensor();
        }
      }

      count_change_state_drop_sensor = 0;
    }
}

extern void CtrlMotorProviderProcessInterrupt(void);

void Perh_ProcessAllWhile(void)
{
    /* Watchdog */
    Wdog_Refesh();

    /* Check trang thai cam bien roi */
    Perh_CheckDropSensorProcess();

    /* write to log */
    Dbg_WriteDataToSDProcess();

    CtrlMotorProviderProcessInterrupt();
}
/**
 * @brief  Ham khoi tao cac ngoai vi
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_Init()
{
    uint8_t data = 0;
    /* Khoi tao chan I/O nhan cam bien cua */
    Door_Init();
    /* Khoi tao chan I/O hien thi led chieu sang*/
    Pin_Init(LED_PIN, PIN_OUTPUT);
    data = 0;
    /* Doc trang thai heater ghi trong ROM */
    bool res = EE_ReadByte(LED_EEP_ADDRESS, &data);
    if (data == 0)
    {
        g_heaterIsOn =  false;
    }
    else if (data == 1)
    {
        g_heaterIsOn = true;
    }

    /* Khoi tao chan I/O va ngat GPIO cho chan cam bien Drop Sensor (cam bien roi) */
    Pin_Init(DROP_SENSOR_PIN, PIN_INPUT);
    PORT_SetPinInterruptConfig(PORTD, 11, kPORT_InterruptEitherEdge);
    NVIC_SetPriority(PORTBCD_IRQn, 0);  /* Highest priority */
    EnableIRQ(PORTBCD_IRQn);
    /* Khoi tao chan I/O cho coi bao */
    Pin_Init(BEEP_PIN, PIN_OUTPUT);

    /* Tien hanh chuyen cac thong tin doc duoc toi trinh xu ly GSM */
    Gsm_SetServerInfo(203, 171, 20, 62, 9201u);

    /* Doc cai dat do am trong ROM */
    g_humidityset = Perh_GetHumidity();

    /* Doc trang thai bat/tat audio trong ROM */
    data = 0;
    EE_ReadByte(AUDIO_EEP_ADDRESS, &data);
    /* Neu du lieu doc duoc = 0 */
    if (data == 0)
    {
        /* Tat am thanh */
        g_audioIsOn = false;
    }
    /* Neu du lieu doc duoc = 1 */
    else if (data == 1)
    {
        /* Bat am thanh */
        g_audioIsOn = true;
    }

    /* Doc trang thai co tra lai tien hay khong trong ROM */
    data = 0;
    EE_ReadByte(REFUND_ONOFF_EEP_ADDRESS, &data);
    /* Neu du lieu doc duoc = 0 */
    if (data == 0)
    {
        /* Cai dat may khong tra lai tien */
        g_refundIsOn = false;
    }
    /* Neu du lieu doc duoc = 1 */
    else if (data == 1)
    {
        /* Cai dat may tra lai tien */
        g_refundIsOn = true;
    }

    /* Doc trang thai bat/tat cua cam bien roi */
    data = 0;
    EE_ReadByte(DROP_SENSOR_EEP_ADDRESS, &data);
    /* Neu du lieu doc duoc = 0 */
    if (data == 0)
    {
        /* Xoa co bao cho cam bien roi */
        g_dropSensorIsOn = false;
    }
    /* Neu du lieu doc duoc = 1 */
    else
    {
        /* Set co bao cho cam bien roi */
        g_dropSensorIsOn = true;
    }
    /* Doc menh gia tien co the nhan */
    data = 0;
    if(EE_ReadByte(ACCEPT_NOTE_MAX_EEP_ADDRESS_LOW, &data))
    {
        g_acceptableNoteLow = data;
    }
    else
    {
      g_acceptableNoteLow = 0xFF;
    }

    if(EE_ReadByte(ACCEPT_NOTE_MAX_EEP_ADDRESS_HIGH, &data))
    {
        g_acceptableNoteHigh = data;
    }
    else
    {
      g_acceptableNoteHigh = 0xFF;
    }

    /* Doc so cot dong co cai dat trong ROM */
    EE_ReadByte(SLOT_COLUMNS_EEP_ADDRESS, &g_slotColumn);
    if(g_slotColumn > 10) g_slotColumn = 10;
    /* Doc so hang dong co cai dat trong ROM*/
    EE_ReadByte(SLOT_ROWS_EEP_ADDRESS, &g_slotRow);
    if(g_slotRow > 6) g_slotRow = 6;
    /* Doc ID cua may cai dat trong ROM (6 so) */
    EE_Read(VEND_ID_EEP_ADDRESS, g_machineID, 6);
    /* Doc cac cai dat ve am luong am thanh cai dat trong ROM */
    EE_ReadByte(CALL_VOLUME_EEP_ADDRESS, &g_callVolume);                /* Am luong cuoc goi */
    EE_ReadByte(MEDIA_VOLUME_EEP_ADDRESS, &g_mediaVolume);              /* Am luong nhac */
    EE_ReadByte(MICROPHONE_LEVEL_EEP_ADDRESS, &g_microphoneLevel);      /* Am luong MIC */
    if (g_callVolume > 10) { g_callVolume = 10; }
    if (g_mediaVolume > 10) { g_mediaVolume = 10; }
    if (g_microphoneLevel > 10) { g_microphoneLevel = 10; }
    /* Doc tinh trang hang hoa (gia, so luong con lai ) trong ROM */
    RestoreItemsFromEeprom();
    /* Cap nhat lai ID(vi tri) cua tung khay san pham */
    Perh_UpdateItemId();
    /* lay ma phien ban hien tai */
    Perh_GetFirmwareVersion(g_FirmwareVersion);

    g_n1_set = Perh_GetN1Time();
    g_n2_set = Perh_GetN2Capacity();
    g_n3_set = Perh_GetN3Capacity();
}
/**
 * @brief  Ham xu ly bat heater va ghi trang thai vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_HeaterOn()
{
    g_heaterIsOn = true;
    EE_WriteByte(LED_EEP_ADDRESS, 1);
}
/**
 * @brief  Ham xu ly tat heater va ghi trang thai vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_HeaterOff()
{
    g_heaterIsOn = false;
    EE_WriteByte(LED_EEP_ADDRESS, 0);
}
/**
 * @brief  Ham doc trang thai bat/tat heater sang tu ROM
 *
 * @param  NONE
 * @retval bool Perh_HeaterIsOn -> Trang thai bat/tat led chieu sang (false = tat, true = bat)
 */
bool Perh_HeaterIsOn()
{
    return g_heaterIsOn;
}
/**
 * @brief  Ham ghi trang thai cam bien roi (bao da roi) vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_DropSensorOn()
{
    EE_WriteByte(DROP_SENSOR_EEP_ADDRESS, 1);
    /* Set co bao cho cam bien roi */
    g_dropSensorIsOn = true;
}
/**
 * @brief  Ham ghi bit cho phep cam bien roi hoat dong vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_DropSensorOff()
{
    EE_WriteByte(DROP_SENSOR_EEP_ADDRESS, 0);
    /* Xoa co bao cho phep cam bien roi hoat dong */
    g_dropSensorIsOn = false;
}
/**
 * @brief  Ham doc bit cho phep cam bien roi hoat dong tu ROM
 *
 * @param  NONE
 * @retval bool Perh_DropSensorIsOn -> Trang thai cam bien (false = chua roi, true = da roi)
 */
bool Perh_DropSensorIsOn()
{
    return g_dropSensorIsOn;
}
/**
 * @brief  Ham ghi trang thai bat am thanh vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_AudioOn()
{
    EE_WriteByte(AUDIO_EEP_ADDRESS, 1);
    g_audioIsOn = true;
}
/**
 * @brief  Ham ghi trang thai tat am thanh vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_AudioOff()
{
    EE_WriteByte(AUDIO_EEP_ADDRESS, 0);
    g_audioIsOn = false;
}
/**
 * @brief  Ham doc trang thai bat/tat am thanh tu ROM
 *
 * @param  NONE
 * @retval bool Perh_AudioIsOn -> Trang thai cam bien (false = tat, true = bat)
 */
bool Perh_AudioIsOn()
{
    return g_audioIsOn;
}
/**
 * @brief  Ham ghi trang thai may cho phep tra lai tien vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_RefundOn()
{
    EE_WriteByte(REFUND_ONOFF_EEP_ADDRESS, 1);
    g_refundIsOn = true;
}
/**
 * @brief  Ham ghi trang thai may khong cho phep tra lai tien vao ROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_RefundOff()
{
    EE_WriteByte(REFUND_ONOFF_EEP_ADDRESS, 0);
    g_refundIsOn = false;
}
/**
 * @brief  Ham doc trang thai tra lai tien tu ROM
 *
 * @param  NONE
 * @retval bool Perh_RefundIsOn -> Trang thai tra lai tien (false = khong tra lai tien, true = dang tra lai tien)
 */
bool Perh_RefundIsOn()
{
    return g_refundIsOn;
}
/**
 * @brief  Ham phat hien san pham da roi
 *
 * @param  NONE
 * @retval bool Perh_ItemIsDetected -> Trang thai san pham roi(false = chua roi, true = da roi)
 */
bool Perh_ItemIsDetected()
{
    return g_dropSensorFlag;
}

/**
 * @brief  Ham xoa co bao cam bien roi
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_ClearDropSensorFlag()
{
    g_dropSensorFlag = false;
}
/**
 * @brief  Ham cai dat ID cho may ban hang
 *
 * @param  uint8_t* id -> Con tro toi ID cua may
 * @retval NONE
 */
void Perh_SetVendId(const uint8_t* id)
{
    EE_Write(VEND_ID_EEP_ADDRESS, id, 6);

    for (int i = 0; i < 6; i++)
    {
        /* Luu ID cua mang vao 1 mang de xu ly */
        g_machineID[i] = id[i];
    }
}
/**
 * @brief  Ham doc lai ID cua may tu mang da luu
 *
 * @param  uint8_t* id -> Con tro toi mang luu ID cua may
 * @retval NONE
 */
void Perh_GetVendId(uint8_t* id)
{
    for (int i = 0; i < 6; i++)
    {
        id[i] = g_machineID[i];
    }
}
/**
 * @brief  Ham cai dat password cap 1 vao ROM
 *
 * @param  uint8_t* password -> Con tro toi password can cai
 * @retval NONE
 */
void Perh_SetShopPassword(uint8_t* password)
{
    EE_Write(SHOP_PASSWORD_EEP_ADDRESS, password, 6);
}
/**
 * @brief  Ham doc password cap 1 tu ROM
 *
 * @param  uint8_t* password -> Con tro toi vi tri luu password
 * @retval NONE
 */
void Perh_GetShopPassword(uint8_t* password)
{
    EE_Read(SHOP_PASSWORD_EEP_ADDRESS, password, 6);
}


/**
 * @brief  Ham cai dat password cap 2 vao ROM
 *
 * @param  uint8_t* password -> Con tro toi password can cai
 * @retval NONE
 */
void Perh_SetTechPassword(uint8_t* password)
{
    EE_Write(TECH_PASSWORD_EEP_ADDRESS, password, 6);
}
/**
 * @brief  Ham doc password cap 2 tu ROM
 *
 * @param  uint8_t* password -> Con tro toi vi tri luu password
 * @retval NONE
 */
void Perh_GetTechPassword(uint8_t* password)
{
    EE_Read(TECH_PASSWORD_EEP_ADDRESS, password, 6);
}

/**
 * @brief  Ham cai dat so cot dong co vao ROM
 *
 * @param  uint8_t* password -> Con tro toi so cot dong co can cai
 * @retval NONE
 */
void Perh_SetSlotColumn(uint8_t columns)
{
    EE_WriteByte(SLOT_COLUMNS_EEP_ADDRESS, columns);
    g_slotColumn = columns;
}
/**
 * @brief  Ham cai dat so hang dong co vao ROM
 *
 * @param  uint8_t* password -> Con tro toi so hang dong co can cai
 * @retval NONE
 */
void Perh_SetSlotRow(uint8_t rows)
{
    EE_WriteByte(SLOT_ROWS_EEP_ADDRESS, rows);
    g_slotRow = rows;
}
/**
 * @brief  Ham tra lai gia tri so cot dong co
 *
 * @param  NONE
 * @retval uint8_t Perh_GetSlotRow -> So cot dong co
 */
uint8_t Perh_GetSlotColumn()
{
    return g_slotColumn;
}
/**
 * @brief  Ham tra lai gia tri so hang dong co
 *
 * @param  NONE
 * @retval uint8_t Perh_GetSlotRow -> So hang dong co
 */
uint8_t Perh_GetSlotRow()
{
    return g_slotRow;
}
/**
 * @brief  Ham doc dia chi IP & Port Server tu ROM
 *
 * @param  uint8_t* ip1 -> Con tro toi vi tri luu IP 1
 * @param  uint8_t* ip2 -> Con tro toi vi tri luu IP 2
 * @param  uint8_t* ip3 -> Con tro toi vi tri luu IP 3
 * @param  uint8_t* ip4 -> Con tro toi vi tri luu IP 4
 * @param  uint8_t* port -> Con tro toi vi tri luu Port
 * @retval NONE
 */
void Perh_GetServerInfo(uint8_t* ip1 ,uint8_t* ip2, uint8_t* ip3, uint8_t* ip4, uint32_t* port)
{
    /* Read server ip address */
    EE_ReadByte(SERVER_IP_EEP_ADDRESS, ip1);
    EE_ReadByte(SERVER_IP_EEP_ADDRESS + 1, ip2);
    EE_ReadByte(SERVER_IP_EEP_ADDRESS + 2, ip3);
    EE_ReadByte(SERVER_IP_EEP_ADDRESS + 3, ip4);

    /* Read server port */
    EE_Read(SERVER_PORT_EEP_ADDRESS, port, 4);
    if (*port >= 36000)
    {
        *port = 0;
    }
}
/**
 * @brief  Ham ghi dia chi IP & Port Server toi ROM
 *
 * @param  uint8_t* ip1 -> Con tro toi IP1 can luu
 * @param  uint8_t* ip2 -> Con tro toi IP2 can luu
 * @param  uint8_t* ip3 -> Con tro toi IP3 can luu
 * @param  uint8_t* ip4 -> Con tro toi IP4 can luu
 * @param  uint8_t* port -> Con tro toi Port can luu
 * @retval NONE
 */
void Perh_SaveServerInfo(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4, uint32_t port)
{
    /* Save server ip address */
    EE_WriteByte(SERVER_IP_EEP_ADDRESS, ip1);
    EE_WriteByte(SERVER_IP_EEP_ADDRESS + 1, ip2);
    EE_WriteByte(SERVER_IP_EEP_ADDRESS + 2, ip3);
    EE_WriteByte(SERVER_IP_EEP_ADDRESS + 3, ip4);

    /* Save server port */
    uint8_t* p = (uint8_t*)(&port);
    EE_Write(SERVER_PORT_EEP_ADDRESS, p, 4);
}
/**
 * @brief  Ham ghi menh gia tien tra lai toi ROM
 *
 * @param  uint8_t channel -> Kenh tien te
 * @param  bool routToPayout -> Bit lua chon set hay reset kenh cai dai (1 = set, 0 = reset)
 * @retval NONE
 */
void Perh_SaveRefundNote(uint8_t channel, bool routToPayout)
{

    if (channel == 0xFF)
    {
        if (routToPayout)
        {
            EE_WriteByte(REFUND_NOTE_EEP_ADDRESS, 0xFF);
        }
        else
        {
            EE_WriteByte(REFUND_NOTE_EEP_ADDRESS, 0x00);
        }
        return;
    }
    /* Neu kenh cai dai > 7 (khong ton tai kenh tien te > 7)*/
    if (channel > 7)
    {
        return;         /* Thoat */
    }

    uint8_t tmp = 0;
    /* Doc lai kenh tien te dang ghi trong ROM vao tmp */
    EE_ReadByte(REFUND_NOTE_EEP_ADDRESS, &tmp);
    /* Neu lua chon la set */
    if (routToPayout)
    {
        /* Dat bit dinh danh cho kenh tien te len 1 (Kenh 1: bit 1, kenh2: bit 2 ...)*/
        tmp |= (1 << channel);
    }
    /* Neu lua chon la reset */
    else
    {
        /* Xoa bit dinh danh cho kenh tien te ve 0 (Kenh 1: bit 1, kenh2: bit 2 ...)*/
        tmp &= ~(1 << channel);
    }
    /* Ghi du lieu tro lai EEPROM */
    EE_WriteByte(REFUND_NOTE_EEP_ADDRESS, tmp);
}
/**
 * @brief  Ham doc menh gia tien tra lai tu ROM
 *
 * @param  uint8_t channe -> Kenh tien te
 * @retval bool Perh_GetRefundNote -> Trang thai thiet lap (false = not set, true = set)
 */
bool Perh_GetRefundNote(uint8_t channel)
{
    /* Neu kenh tien te > 7, khong ton tai kenh > 7*/
    if (channel > 7)
    {
        return false;
    }
    uint8_t tmp = 0;
    /* Doc du lieu ghi trong ROM */
    EE_ReadByte(REFUND_NOTE_EEP_ADDRESS, &tmp);
    /* Kiem tra bit dinh danh cho kenh tien te (Kenh 1: bit 1, kenh2: bit 2 ...)*/
    if (tmp & (1 << channel))
    {
        /* Neu bit = 1 */
        return true;
    }
    /* Neu bit = 0 */
    return false;
}
/**
 * @brief  Ham ghi do am cai dat toi ROM
 *
 * @param  uint8_t humidity -> Do am cai dat
 * @retval NONE
 */
void Perh_SetHumidity(uint8_t humidity)
{
    EE_WriteByte(HUMIDITY_EEP_ADDRESS, humidity);
}
/**
 * @brief  Ham doc do am cai dat tu ROM
 *
 * @param  NONE
 * @retval uint8_t Perh_GetHumidity -> Do am cai dat tra lai tu ROM
 */
uint8_t Perh_GetHumidity()
{
    uint8_t data = 0;
    EE_ReadByte(HUMIDITY_EEP_ADDRESS, &data);
    return data;
}
/**
 * @brief  Ham lay menh gia tien lon nhat duoc chap nhan (1K - 200K)
 *
 * @param  NONE
 * @retval Menh gia tien ( 0 - 50.000VND, 1 - 100.000VND, 2 - 200.000VND)
 */
uint8_t Perh_GetAcceptableNoteLow()
{
    return g_acceptableNoteLow;
}

/**
 * @brief  Ham lay menh gia tien lon nhat duoc chap nhan (500K)
 *
 * @param  NONE
 * @retval Menh gia tien ( 500.000VND)
 */
uint8_t Perh_GetAcceptableNoteHight()
{
    return g_acceptableNoteHigh;
}
/**
 * @brief  Ham luu menh gia tien lon nhat duoc chap nhan toi ROM
 *
 * @param  uint8_t note -> Menh gia tien ( 0 - 50.000VND, 1 - 100.000VND, 2 - 200.000VND)
 * @retval NONE
 */
void Perh_SaveAcceptableNoteLow(uint8_t note)
{
    EE_WriteByte(ACCEPT_NOTE_MAX_EEP_ADDRESS_LOW, note);
    g_acceptableNoteLow = note;
}

/**
 * @brief  Ham luu menh gia tien lon nhat duoc chap nhan toi ROM
 *
 * @param  uint8_t note -> Menh gia tien ( 500.000VND)
 * @retval NONE
 */
void Perh_SaveAcceptableNoteHigh(uint8_t note) {
    EE_WriteByte(ACCEPT_NOTE_MAX_EEP_ADDRESS_HIGH, note);
    g_acceptableNoteHigh = note;
}
/**
 * @brief
 *
 * @param
 * @retval NONE
 */
void Perh_SaveBalance(uint32_t balance)
{
    uint8_t* p = (uint8_t*)(&balance);

    EE_Write(STORED_BALANCE_EEP_ADDRESS, p, 4);
}
/**
 * @brief  Ham luu tong so tien da nhan duoc toi ROM
 *
 * @param  uint32_t receivedNotes -> Tong so tien can lu vao ROM
 * @retval NONE
 */
void Perh_SaveReceivedNotes(uint32_t receivedNotes)
{
    uint8_t* p = (uint8_t*)(&receivedNotes);

    EE_Write(STORED_RECEIVED_NOTES_EEP_ADDRESS, p, 4);
}
/**
 * @brief
 *
 * @param
 * @retval NONE
 */
void Perh_RestoreBalance(uint32_t* balance)
{
    EE_Read(STORED_BALANCE_EEP_ADDRESS, balance, 4);

    if (*balance == 0xFFFFFFFF)
    {
        *balance = 0;
    }
}
/**
 * @brief  Ham doc tong so tien da nhan duoc luu trong ROM
 *
 * @param  uint32_t* receivedNotes -> Con tro luu tong so tien nhan duoc
 * @retval NONE
 */
void Perh_RestoreReceivedNotes(uint32_t* receivedNotes)
{
    EE_Read(STORED_RECEIVED_NOTES_EEP_ADDRESS, receivedNotes, 4);

    if (*receivedNotes == 0xFFFFFFFF)
    {
        *receivedNotes = 0;
    }
}
/**
 * @brief  Ham ghi them so tien vua nhan vao (dua vao may)
 *
 * @param  uint32_t amount -> So tien vua dua vao
 * @retval NONE
 */
void Perh_AddNoteIn(uint32_t amount)
{
    /* Lay tong so tien da dua vao truoc do luu trong ROM */
    uint32_t totalNoteIn = Perh_GetTotalNoteIn();
    /* Cong them so tien vua dua vao */
    totalNoteIn += amount;
    uint8_t* p = (uint8_t*)(&totalNoteIn);
    /* Luu vao ROM */
    EE_Write(MONEY_IN_EEP_ADDRESS, p, 4);
    /* Kiem tra trang thai ghi */
    /* Doc lai  tong so tien trong ROM va so sanh voi tong so tien can ghi*/
    while (Perh_GetTotalNoteIn() != totalNoteIn)
    {
        /* Neu khac nhau, tien hanh ghi lai lan nua */
        EE_Write(MONEY_IN_EEP_ADDRESS, p, 4);
        Delay(1);
    }
}
/**
 * @brief  Ham ghi them so tien vua xuat ra (tra lai khach)
 *
 * @param  uint32_t amount -> So tien vua xuat ra
 * @retval NONE
 */
void Perh_AddNoteOut(uint32_t amount)
{
    /* Doc lai tong so tien da xuat ra truoc do trong ROM */
    uint32_t totalNoteOut = Perh_GetTotalNoteOut();
    /* Cong them so tien vua xuat */
    totalNoteOut += amount;
    uint8_t* p = (uint8_t*)(&totalNoteOut);
    /* Luu vao ROM */
    EE_Write(MONEY_OUT_EEP_ADDRESS, p, 4);
    /* Kiem tra trang thai ghi */
    /* Neu tong so tien xuat ra doc trong ROM khac tong so tien can ghi vao */
    while (Perh_GetTotalNoteOut() != totalNoteOut)
    {
        /* Neu khac nhau, tien hanh ghi lai lan nua */
        EE_Write(MONEY_OUT_EEP_ADDRESS, p, 4);
        Delay(1);
    }
}
/**
 * @brief  Ham xoa toan bo so tien nhan vao va xuat ra trong ROM (reset)
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_ResetNoteInOut()
{
    uint32_t tmp = 0;

    uint8_t* p = (uint8_t*)(&tmp);
    EE_Write(MONEY_IN_EEP_ADDRESS, p, 4);
    EE_Write(MONEY_OUT_EEP_ADDRESS, p, 4);
}
/**
 * @brief  Ham lay gia tri tong so tien da nhan vao (dua vao may)
 *
 * @param  NONE
 * @retval uint32_t -> So tien da nhan vao may
 */
uint32_t Perh_GetTotalNoteIn()
{
    uint32_t totalNoteIn = 0, count = 0;
    /* Doc tu ROM */
    EE_Read(MONEY_IN_EEP_ADDRESS, &totalNoteIn, 4);
    /* Neu qua gioi han */
    if (totalNoteIn == 0xFFFFFFFF)
    {
        totalNoteIn = 0;
        return 0;
        /* Tra ket qua la 0 */
    }
    /* Neu so tien doc duoc bi le (So tien co gia tri tram dong. vd: 15.200VND */
    while ((totalNoteIn % 1000) != 0)
    {
        /* Tien hanh doc lai ket qua 4 lan*/
        totalNoteIn = 0;
        EE_Read(MONEY_IN_EEP_ADDRESS, &totalNoteIn, 4);
        Delay(1);
        count++;
        if (count > 3)
        {
            break;
        }
        /* Doc sau 4 lan so tien nhan duoc van le -> thoat*/
    }
    return totalNoteIn;
}
/**
 * @brief  Ham lay gia tri tong so tien xuat ra (tra lai)
 *
 * @param  NONE
 * @retval uint32_t -> So tien da xuat ra
 */
uint32_t Perh_GetTotalNoteOut()
{
    uint32_t totalNoteOut = 0, count = 0;
    /* Doc tu ROM */
    EE_Read(MONEY_OUT_EEP_ADDRESS, &totalNoteOut, 4);
    /* Neu qua gioi han */
    if (totalNoteOut == 0xFFFFFFFF)
    {
        totalNoteOut = 0;
        return 0;
        /* Tra ket qua la 0 */
    }
    /* Neu so tien doc duoc bi le (So tien co gia tri tram dong. vd: 15.200VND */
    while ((totalNoteOut % 1000) != 0)
    {
        /* Tien hanh doc lai ket qua 4 lan*/
        totalNoteOut = 0;
        EE_Read(MONEY_OUT_EEP_ADDRESS, &totalNoteOut, 4);
        Delay(1);
        count++;
        if (count > 3)
        {
            break;
        }
        /* Doc sau 4 lan so tien nhan duoc van le -> thoat*/
    }

    return totalNoteOut;
}

/**
 * @brief  Ham lay so goi tong dai da luu trong ROM
 *
 * @param  char* number -> Con tro toi vi tri luu so dien thoai
 * @retval Trang thai doc (False = doc loi, True = doc ok)
 */
bool Perh_GetOperatorNumber(char* number)
{
    uint8_t failCount = 0;

    while (1)
    {
        /* Doc EEPROM neu ok -> thoat */
        if (EE_Read(OP_NUM_EEP_ADDRESS, number, 16) == true)
        {
            break;
        }
        else
        {
            /* Neu khong ok -> Thu doc lai 5 lan */
            failCount++;
            if (failCount > 5)
            {
                /* Bao doc loi */
                return false;
            }
        }
    }
    /* Bao doc ok */
    return true;
}
/**
 * @brief  Ham ghi so goi tong dai toi ROM
 *
 * @param  char* number -> Con tro toi so dien thoai can luu
 * @retval Trang thai ghi (False = ghi loi, True = ghi ok)
 */
bool Perh_SetOperatorNumber(const char* number)
{
    int i = 0;
    uint8_t failCount = 0;
    uint8_t readData[16];
    uint8_t* p = (uint8_t*)number;
    /* Ghi so dien thoai vao ROM */
    EE_Write(OP_NUM_EEP_ADDRESS, p, 16);
    /* Kiem tra lai trang thai ghi*/
    while (1)
    {
        /* Doc lai so dien thoat */
        Perh_GetOperatorNumber(readData);
        for (i = 0; i < 16; i++)
        {
            /* So sanh tung so dien thoai doc doc voi so dien thoai ghi vao */
            if (readData[i] != p[i])
            {
                /* Neu co so khac nhau -> tien hanh ghi lai */
                EE_Write(OP_NUM_EEP_ADDRESS, p, 16);
                Delay(1);
                failCount++;
                /* Neu ghi qua 5 lan */
                if (failCount > 5)
                {
                    /* Bao ghi loi */
                    return false;
                }
                break;
            }
        }
        if (i == 16)
        {
            /* Bao ghi ok */
            return true;
        }
    }
}
/**
 * @brief  Ham doc gia tri cai dat am luong cuoc goi
 *
 * @param  NONE
 * @retval g_callVolume -> Gia tri am luong cuoc goi
 */
uint8_t Perh_GetCallVolume()
{
    return g_callVolume;
}
/**
 * @brief  Ham ghi gia tri cai dat am luong cuoc goi vao ROM
 *
 * @param  uint8_t value -> Gia tri cai dat am luong
 * @retval NONE
 */
void Perh_SetCallVolume(uint8_t value)
{
    if (value > 10)
    {
        value = 10;
    }

    EE_WriteByte(CALL_VOLUME_EEP_ADDRESS, value);
    g_callVolume = value;
}
/**
 * @brief  Ham doc gia tri cai dat am luong media
 *
 * @param  NONE
 * @retval g_mediaVolume -> Gia tri am luong media
 */
uint8_t Perh_GetMediaVolume()
{
    return g_mediaVolume;
}
/**
 * @brief  Ham ghi gia tri cai dat am luong media vao ROM
 *
 * @param  uint8_t value -> Gia tri cai dat am luong
 * @retval NONE
 */
void Perh_SetMediaVolume(uint8_t value)
{
    if (value > 10)
    {
        value = 10;
    }

    EE_WriteByte(MEDIA_VOLUME_EEP_ADDRESS, value);
    g_mediaVolume = value;
}
/**
 * @brief  Ham doc gia tri cai dat am luong MIC
 *
 * @param  NONE
 * @retval g_microphoneLevel -> Gia tri am luong MIC
 */
uint8_t Perh_GetMicrophoneLevel()
{
    return g_microphoneLevel;
}
/**
 * @brief  Ham ghi gia tri cai dat am luong MIC vao ROM
 *
 * @param  uint8_t value -> Gia tri cai dat am luong
 * @retval NONE
 */
void Perh_SetMicrophoneLevel(uint8_t value)
{
    if (value > 10)
    {
        value = 10;
    }

    EE_WriteByte(MICROPHONE_LEVEL_EEP_ADDRESS, value);
    g_microphoneLevel = value;
}
/**
 * @brief  Ham cap nhat lai vi tri (ID) hang hoa
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_UpdateItemId()
{
    int i, j;
    int count = 0;
    uint8_t rows = Perh_GetSlotRow();
    uint8_t columns = Perh_GetSlotColumn();

    for (i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        g_itemList[i].id = 0;
    }

    if (rows > 9)  rows = 9;
    if (columns > 10)  columns = 10;

    for (i = 0; i < rows; i++)
    {
        for (j = 1; j <= columns; j++)
        {
            g_itemList[count].id = count + 1;
            count++;
        }
    }
}
/**
 * @brief  Ham cai dat menh gia cho tung vi tri hang hoa tren khay
 *
 * @param  int id -> Vi tri hang hoa (ID)
 * @param  uint32_t price -> Menh gia hang hoa
 * @retval NONE
 */
void Perh_SetItemPrice(int id, uint32_t price)
{
    if (id <= 0)    return;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            g_itemList[i].price = price;
            break;
        }
    }
}
/**
 * @brief  Ham cai dat so luong hang tren 1 khay
 *
 * @param  int id -> Vi tri hang hoa (ID)
 * @param  uint8_t number -> So luong hang tai vi tri (tren 1 khay)
 * @retval NONE
 */
void Perh_SetItemNumber(int id, uint8_t number)
{
    if (id <= 0)    return;

    if (number > 100)
    {
        number = 100;
    }

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            g_itemList[i].available = number;
            break;
        }
    }
}
/**
 * @brief  Ham tra ve con tro chua toan bo thong tin cua khay hang hoa
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval &(g_itemList[i]) -> Con tro chua toan bo thong tin cua khay hang
 */
item_t* Perh_GetItem(int id)
{
    if (id <= 0)    return NULL;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            return &(g_itemList[i]);
        }
    }

    return NULL;
}
/**
 * @brief  Ham lay thong tin menh gia cua hang hoa tren 1 khay
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval g_itemList[i].price -> Menh gia hang hoa tren khay
 */
uint32_t Perh_GetItemPrice(int id)
{
    if (id <= 0)    return 0;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            return g_itemList[i].price;
        }
    }

    return 0;
}
/**
 * @brief  Ham lay thong tin so luong hang hoa tren 1 khay
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval g_itemList[i].available -> So luong hang hoa con lai tren khay
 */
uint8_t Perh_GetItemNumber(int id)
{
    if (id <= 0)    return 0;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            return g_itemList[i].available;
        }
    }

    return 0;
}
/**
 * @brief  Ham cai dat khoa khay hang hoa
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval NONE
 */
void Perh_LockItem(int id)
{
    if (id <= 0)    return;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            g_itemList[i].isLocked = true;
            break;
        }
    }
}
/**
 * @brief  Ham cai dat mo khoa khay hang hoa
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval NONE
 */
void Perh_UnlockItem(int id)
{
    if (id <= 0)    return;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            g_itemList[i].isLocked = false;
            break;
        }
    }
}
/**
 * @brief  Ham tra ve trang thai khay hang co bi khoa hay khong
 *
 * @param  NONE
 * @retval g_itemList[i].isLocked -> Trang thai khay hang (True = khoa, False = mo)
 */
bool Perh_ItemIsLocked(int id)
{
    if (id <= 0)    return true;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            return g_itemList[i].isLocked;
        }
    }
    return true;
}
/**
 * @brief  Ham kiem tra khay hang hoa co ton tai hay khong
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval bool -> Tinh trang ton tai cua khay hang (True = co, False = khong)
 */
bool Perh_ItemIsExist(int id)
{
    if (id <= 0)    return false;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            return true;
        }
    }

    return false;
}
/**
 * @brief  Ham kiem tra khay hang hoa co con hang hay khong
 *
 * @param  int id -> Vi tri khay hang hoa (ID)
 * @retval bool -> Tinh trang con hang tren khay (True = con hang, False = het hang)
 */
bool Perh_ItemIsAvailable(int id)
{
    if (id <= 0)    return false;

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        if (g_itemList[i].id == id)
        {
            if (g_itemList[i].available > 0)
            {
                return true;
            }
            break;
        }
    }

    return false;
}

/**
 * @brief  Ham luu toan bo thong tin hang hoa tu mang g_itemList toi EEPROM
 *
 * @param  NONE
 * @retval NONE
 */
void Perh_SaveItemsToEeprom()
{
    /* Lay do rong bufer theo tong so khay hang (moi khay hang gom 03 dia chi buffer */
    uint32_t buffer[RESOURCE_ITEM_MAX * 3 + 3];

    /* Chuan bi du lieu de ghi */
    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        buffer[i * 3] = g_itemList[i].price;
        buffer[i * 3 + 1] = g_itemList[i].available;
        buffer[i * 3 + 2] = g_itemList[i].isLocked;
    }
    /* Moi item gom 3 buffer, moi buffer gom 4 byte */
    uint32_t size = RESOURCE_ITEM_MAX * 12;     /* Do dai du lieu ghi vao ROM = Tong so khay x 3 x 4*/
    uint8_t* p = (uint8_t*)buffer;

    for (int i = 0; i < size; i++)
    {
        uint8_t data;
        /* Doc lan luot tung byte du lieu hang hoa da ghi trong ROM truoc do bat dau tu dia chi ITEM_INFO_EEP_ADDRES */
        bool res = EE_ReadByte(ITEM_INFO_EEP_ADDRESS + i, &data);
        /* Neu doc bat ki byte nao xuat hien loi */
        if (res == false)
        {
            sprintf(bufferDebug, "Eeprom >> Error to read items from eeprom: i = %d", i);
            Dbg_Println(bufferDebug);
            return;     /* -> Thoat */
        }
        /* Kiem tra tung byte doc duoc so voi byte can ghi (Xem hang hoa co su thay doi khong) */
        if (data != p[i])
        {
            /* Neu co su thay doi ve du lieu hang hoa -> Tien hanh ghi ROM du lieu thay doi nayf */
            if (EE_WriteByte(ITEM_INFO_EEP_ADDRESS + i, p[i]) == false)
            {
                /* Neu ghi loi -> In man hinh debug */
                sprintf(bufferDebug, "Eeprom >> Error to write items to eeprom: i = %d", i);
                Dbg_Println(bufferDebug);
                return;
            }
        }
    }

}
/**
 * @brief  Ham doc lai toan bo thong tin hang hoa tu EEPROM vao mang g_itemList
 *
 * @param  NONE
 * @retval NONE
 */
static void RestoreItemsFromEeprom()
{
    uint32_t buffer[RESOURCE_ITEM_MAX * 3 + 3];

    if (EE_Read(ITEM_INFO_EEP_ADDRESS, (uint8_t*)buffer, RESOURCE_ITEM_MAX * 12) == false)
    {
        Dbg_Println("Eeprom >> Error to read the items from eeprom");
        return;
    }

    for (int i = 0; i < RESOURCE_ITEM_MAX; i++)
    {
        g_itemList[i].price = buffer[i * 3];
        g_itemList[i].available = buffer[i * 3 + 1];
        g_itemList[i].isLocked = buffer[i * 3 + 2];
    }
}

/**
 * @brief  Ham doc firmware version dang hoat dong ghi trong eeprom
 *
 * @param  uint8_t* fir_ver dia chi con tro lay ten firmware
 * @retval NONE
 */
void Perh_GetFirmwareVersion(uint8_t* fir_ver)
{
  uint8_t lengNameVersion;
  uint8_t nameVersion[20] = {0};
  uint8_t posDot[3] = {0};
  uint8_t countDot = 0;
  /* Read length of version name */
  EE_Read(LENGTH_FILE_UPDATE_NAME_ADDRESS, &lengNameVersion, 1);
  if(lengNameVersion > 20) lengNameVersion = 20;
  /* Read name of version */
  EE_Read(VERSION_CURRENT, nameVersion, lengNameVersion);
  /* Find dots in name */
  for(uint8_t count = 0; count < lengNameVersion; count++)
  {
    if(nameVersion[count] == '.')
    {
      posDot[countDot++] = count;
    }
  }
  /* byte first of version */
  if(posDot[0] == 4) fir_ver[0] = nameVersion[3] - '0';
  else if(posDot[0] == 5) fir_ver[0] = (nameVersion[3] - '0')*10 + (nameVersion[4] - '0');
  /* byte second of version */
  if((posDot[1] - posDot[0]) == 2) fir_ver[1] = nameVersion[posDot[0] + 1] - '0';
  else if((posDot[1] - posDot[0]) == 3) fir_ver[1] = (nameVersion[posDot[0] + 1] - '0')*10 + nameVersion[posDot[0] + 2] - '0';
  /* byte third of version */
  if((posDot[2] - posDot[1]) == 2) fir_ver[2] = nameVersion[posDot[1] + 1] - '0';
  else if((posDot[2] - posDot[1]) == 3) fir_ver[2] = (nameVersion[posDot[1] + 1] - '0')*10 + nameVersion[posDot[1] + 2] - '0';
}

/**
 * @brief  Viet do dai ten firmware dang hoat dong
 *
 * @param  uint8_t* fir_ver dia chi con tro lay ten firmware
 * @retval NONE
 */
void Perh_InitFirmwareVersionName(uint8_t* fir_ver, uint8_t lenName)
{
  EE_WriteByte(LENGTH_FILE_UPDATE_NAME_ADDRESS, lenName);
  EE_Write(VERSION_CURRENT, fir_ver, lenName);
}

/**
 * @brief  Ham ghi trang thai loi cua payout
 *
 * @param  bool state -> Trang thai loi cua dau tra lai tien
 * @retval NONE
 */
void Perh_SetStateErrorPayout(bool state)
{
    if(state == true)
    {
        EE_WriteByte(STATE_ERROR_PAYOUT_ADDRESS, (uint8_t)0x01);
    }
    else
    {
        EE_WriteByte(STATE_ERROR_PAYOUT_ADDRESS, (uint8_t)0x00);
    }
}

bool Perh_GetStateErrorPayout()
{
    uint8_t data = 0;
    EE_ReadByte(STATE_ERROR_PAYOUT_ADDRESS, &data);
    if(data == 0x01)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Perh_SetErrorDropSensor(void)
{
    EE_WriteByte(STATE_ERROR_DROP_SENDOR, (uint8_t)0x01);
}

void Perh_ClearErrorDropSensor(void)
{
    EE_WriteByte(STATE_ERROR_DROP_SENDOR, (uint8_t)0x00);
}

uint8_t Perh_GetErrorDropSensor(void)
{
    uint8_t data = 0;
    EE_ReadByte(STATE_ERROR_DROP_SENDOR, &data);
    if(data == 0xFF) return 0x00;
    return data;
}

void Perh_SetCountFileDebug(uint32_t count_file)
{
    uint32_t tmp, failCount = 0;
    /* Lay dia chi cua con tro luu vi tri du lieu */
    uint8_t* p = (uint8_t*)(&count_file);
    /* Ghi du lieu vao EEPROM, do dai 4 byte */
    EE_Write(COUNT_FILE_DEBUG, p, 4);
    /* Kiem tra lai qua trinh ghi */
    while (1)
    {
        tmp = 0;
        /* Doc lai du lieu vua ghi vao tmp */
        EE_Read(COUNT_FILE_DEBUG, &tmp, 4);
        /* Neu du lieu can ghi va du lieu doc ve khac nhau */
        if (tmp != count_file)
        {
            failCount++;
            /* Neu so lan ghi loi va ghi lai qua 50 lan -> thoat */
            if (failCount > 50)
            {
                break;
            }
            /* Thuc hien ghi lai */
            EE_Write(COUNT_FILE_DEBUG, p, 4);
        }
        /* Neu du lieu ghi ok -> thoat */
        else
        {
            break;
        }
    }
}
uint32_t Perh_GetCountFileDebug(void)
{
    uint8_t failCount = 0;
    uint32_t tmp[5];
    uint32_t value = 0x00;
    uint8_t i = 0;
    /* Thiet lap lai vi tri du lieu ghi trong EEPROM */
    while (1)
    {
        /* Xoa toan bo tmp */
        for (i = 0; i < 5; i++)
        {
            tmp[i] = 0;
        }
        /* Tien hanh doc 5 lan vi tri du lieu ghi trong EEPROM */
        for (i = 0; i < 5; i++)
        {
            EE_Read(COUNT_FILE_DEBUG, &tmp[i], 4);
            Delay(50);
            if (i > 0)
            {
                /* Kiem tra neu du lieu vua doc duoc khac lan doc truoc */
                if (tmp[i] != tmp[i - 1])
                {
                    failCount++;        /* Dem so lan khac */
                    if (failCount > 4)
                    {
                        break;
                    }
                    i = 0;
                }
            }
        }
        /* Neu du lieu qua moi lan doc khac nhau qua 10 lan*/
        if (failCount > 4)
        {
            return 0x00;
        }
        /* Neu du lieu doc duoc tu EEPROM la ok (khac nhau khong qua 10 lan) */
        else
        {
            /* Nhan vi tri du lieu ghi tu EEPROM */
            value = tmp[0];
            if (value == 0xFFFFFFFF)
            {
                /* Thiet lap vi tri du lieu ghi ve vi tri ban dau */
                value = 0x00;
                break;
            }
            else
            {
                break;
            }
        }
    }
    return value;
}

uint32_t Perh_GetN1Time() {
    uint32_t buf = 0;
    EE_Read(N1_TIME_ADDRESS, &buf, 4);
    if(buf > 60) buf = 60;
    if(buf < 1) buf = 1;
    return buf;
}

void Perh_SetN1Time(uint32_t u32N1Time) {
    uint8_t* p = (uint8_t*)(&u32N1Time);
    EE_Write(N1_TIME_ADDRESS, p, 4);
}

uint32_t Perh_GetN2Capacity() {
    uint32_t buf = 0;
    EE_Read(N2_CAPACITY_ADDRESS, &buf, 4);
    if(buf > 1000) buf = 1000;
    if(buf < 1) buf = 1;
    return buf;
}

void Perh_SetN2Capacity(uint32_t u32N2) {
    uint8_t* p = (uint8_t*)(&u32N2);
    EE_Write(N2_CAPACITY_ADDRESS, p, 4);
}

uint32_t Perh_GetN3Capacity() {
    uint32_t buf = 0;
    EE_Read(N3_CAPACITY_ADDRESS, &buf, 4);
    if(buf > 1000) buf = 1000;
    if(buf < 1) buf = 1;
    return buf;
}

void Perh_SetN3Capacity(uint32_t u32N3) {
    uint8_t* p = (uint8_t*)(&u32N3);
    EE_Write(N3_CAPACITY_ADDRESS, p, 4);
}
