
#include "nv11.h"
#include "ITLSSPProc.h"
#include "SSPComs.h"
#include "ssp_helpers.h"
#include "wav_player.h"
#include "peripheral.h"
 /* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_NV11
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif

static SSP_COMMAND g_sspCommand;
static bool g_initialized = false;              // Trang thai hoan thanh viec cai dat
static uint32_t g_oldTick = 0;
static bool g_isHolding = false;                // Trang thai giu (ngam) tien duoc xac lap hay khong
static nv11_event_t g_latestEvent = NV11_EVENT_NONE;
static uint32_t g_latestNote = 0;               // Menh gia to tien gan nhat
static uint8_t g_unitType = 0;                  // Loai dau doc tien

static SSP6_SETUP_REQUEST_DATA setup_req;

static int g_denominations[9] = { 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000 };

static void parse_poll(SSP_COMMAND *sspC, SSP_POLL_DATA6 * poll);

static uint32_t g_channels[10];                 // Menh gia tien trong tu kenh tien te
static uint8_t g_channelNumber = 0;
static uint32_t g_storedNotes[10];              // Tong so tien cua tung menh gia trong FloatNote -> NV200
static bool     g_refundChannels[10];           // Kenh tien te cho phep tra lai -> voi NV200
static uint32_t g_refundNote = 0;               // Kenh tien te cho phep tra lai -> voi NV11
uint16_t g_countInitPayoutFail = 0;             // Dem so lan init dau doc tien bi fail (init khi khoi dong may)
bool g_initPayoutFault = false;
bool g_ActionPayoutFault = false;
extern uint32_t g_numNotePayout;
extern uint32_t g_lastNumNotePayout;
extern bool g_errorPayout;
extern int g_holdBalance;
extern int g_storedBalance;
extern bool g_resetNv11State;

//extern uint8_t flag_ctrl_motor_provider; //Them 4/3/2025

extern void updateCountDownReject(uint8_t value);
extern void updateCountDownPayout(uint8_t value);
/**
 * @brief   Ham khoi tao dau doc tien NV11
 *
 * @param   NONE
 * @retval  int NV11_Init -> Tra ve trang thai khoi tao dau doc tien (0 = hoan thanh init, 1 = loi init)
 */
int NV11_Init(void)
{
    SSP_COMMAND* sspC = &g_sspCommand;
    unsigned int i = 0;
    sspC->PortNumber = 1;
    sspC->SSPAddress = 0;
    sspC->Timeout = 300;
    sspC->EncryptionStatus = NO_ENCRYPTION;
    sspC->RetryLevel = 3;
    sspC->BaudRate = 9600;

    for (i = 0; i < 10; i++)
    {
        g_refundChannels[i] = false;                            /* Xoa toan bo du lieu trong refundChannel */
    }

    /* Kiem tra su co mat cua NV11 (dau NV11 co duoc ket noi voi MCU khong) */
    Dbg_Println("NV11 >> Getting information from Note Validator...");
    /* Ham kiem tra dong bo tra ket qua = 0xF0 -> Tim thay thiet bi */
    if (ssp6_sync(sspC) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> NO VALIDATOR FOUND");
        return 1;
    }
    Dbg_Println ("NV11 >> Validator Found");

    /* Ham cai dat khoa ma hoa (Encryption key) mac dinh -> Ket qua phan hoi ve phai = 0xF0 (cai dat ok) */
    if (ssp6_setup_encryption(sspC,(unsigned long long)0x123456701234567LL) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Encryption Failed");
        return 1;
    }
    else
    {
        Dbg_Println("NV11 >> Encryption Setup");
    }
    /* Ham kiem tra Version cua SSP co phai V6 -> Ham tra ve xac nhan la V6 = 0xF0 */
    if (ssp6_host_protocol(sspC, 0x06) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Host Protocol Failed");
        return 1;
    }

    /* Ham thu thap toan bo cac gia tri cai dat trong thiet bi (bao gom UnitType =0x07 (NV11), Firmwareversion (03760), Numberofchannel (6) ,
       channeldata = Menh gia tien trong kenh, RealValueMultifier = 100, ProtocolVersion = 6*/
    if (ssp6_setup_request(sspC, &setup_req) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Setup Request Failed");
        return 1;
    }
    /* In ra man hinh debug Vision cua phan mem, va kenh tien te */
    sprintf(bufferDebug, "NV11 >> Firmware: %s", setup_req.FirmwareVersion);
    Dbg_Println(bufferDebug);
    Dbg_Println("NV11 >> Channels:");
    g_channelNumber = setup_req.NumberOfChannels;
    /* In ra man hinh menh gia tien cua tung kenh tien te: 1 = 1.000, 2 = 2.000, 3 = 5.000, 4 = 10.000, 5 = 20.000, 6 = 50.000*/
    for (i = 0; i < setup_req.NumberOfChannels; i++)
    {
        g_channels[i] = setup_req.ChannelData[i].value;
        sprintf(bufferDebug, "NV11 >> channel %d: %d %s", i+1, setup_req.ChannelData[i].value, setup_req.ChannelData[i].cc);
        Dbg_Println(bufferDebug);
    }

    /* Cho phep dau doc tien hoat dong -> Ket qua ok = 0xF0 */
    if (ssp6_enable(sspC) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Enable Failed");
        return 1;
    }
    /* Neu UnitType = 0x03 -> Tuc la thiet bi dang ket noi la SMART Hopper*/
    if (setup_req.UnitType == 0x03)
    {
        /* SMART Hopper requires different inhibit commands */
        for (i = 0; i < setup_req.NumberOfChannels; i++)
        {
            /* Ham cai dat cac kenh tien te duoc phep tra lai */
            ssp6_set_coinmech_inhibits(sspC, setup_req.ChannelData[i].value, setup_req.ChannelData[i].cc, ENABLED);
        }
    }
    /* Neu la cac loai dau doc tien khac */
    else
    {
        /* 0x06 -> dau doc SMART Payout, 0x07 -> dau doc NV11*/
//        if (setup_req.UnitType == 0x06 || setup_req.UnitType == 0x07)
//        {
//            /* Cho phep kich hoat tra lai tien -> ket qua ok = 0xF0 */
//            if (ssp6_enable_payout(sspC, setup_req.UnitType) != SSP_RESPONSE_OK)
//            {
//                Dbg_Println("NV11 >> Enable payout Failed");
//                g_countInitPayoutFail++;
//                if(g_countInitPayoutFail == 5)
//                {
//                  g_unitType = setup_req.UnitType;
//                  g_initialized = 1;
//                }
//                return 1;
//            }
//        }

        g_unitType = setup_req.UnitType;        /* Nhan chung loai dau doc tin vao g_unitType */

        /* Cai dat cho phep tat ca cac kenh tien te hoat dong (6 kenh)
           Vd: Neu cho phep kenh 1 - 3, khoa kenh 4 - 16: data = 0x07,0x00 */
        if (ssp6_set_inhibits(sspC, Perh_GetAcceptableNoteLow(), Perh_GetAcceptableNoteHight()) != SSP_RESPONSE_OK)
        {
            Dbg_Println("NV11 >> Inhibits Failed");
            return 1;
        }
        /* Neu dau doc tien la loai NV11 */
        if (g_unitType == NV_TYPE_NV11)
        {
            /* In ra man hinh chung loai dau doc NV11 */
            Dbg_Println("NV11 >> Unit Type: NV11");
            /* Lay so to tien chua trong FloatNote (dau tra lai) hien thi len man hinh */
            int storedNotes = ssp6_get_stored_notes(&g_sspCommand);
            sprintf(bufferDebug, "NV11 >> Stored Notes: %d", storedNotes);
            Dbg_Println(bufferDebug);

            g_lastNumNotePayout = storedNotes;
            g_numNotePayout = storedNotes;

            /* xoa loi payout note */
            g_errorPayout = false;
            Perh_SetStateErrorPayout(g_errorPayout);

        }
        /* Neu dau doc tien la NV200 */
        else if (g_unitType == NV_TYPE_NV200)
        {
            /* In ra man hinh chung loai dau doc Smart Payout */
            Dbg_Println("NV11 >> Unit Type: Smart Payout");
            Dbg_Println("NV11 >> Stored Notes:");
            /* Lay so to tien cua tung loai tien te cai dat trong FloatNote (dau tra lai), hien thi ra man hinh */
            for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
            {
                g_storedNotes[channelIndex] = ssp6_check_note_level(&g_sspCommand, g_channels[channelIndex]);
                sprintf(bufferDebug, "%d: %d", g_channels[channelIndex], g_storedNotes[channelIndex]);
                Dbg_Println(bufferDebug);
            }
        }
        /* Neu dau doc la NV9 (khong co dau tra lai tien)*/
        else if (g_unitType == NV_TYPE_NV9)
        {
            /* In ra man hinh chung loai dau doc NV9 */
            Dbg_Println("NV11 >> Unit Type: NV9");
            Perh_RefundOff();
        }
        /* Neu la cac loai dau doc khac */
        else
        {
            /* In ra man hinh khong nhan dang duoc dau doc */
            Dbg_Println("NV11 >> Unit Type: UNKNOWN");
            return 1;
        }
        /* Neu dau doc tien la loai NV11 */
        if (g_unitType == NV_TYPE_NV11)
        {
            for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
            {
                /* Lay kenh tien te cho phep tra lai cai dat trong ROM */
                if (Perh_GetRefundNote(channelIndex) == true)
                {
                    /* Cai dat menh gia tien te luu tru tren FloatNote de tra lai */
                    if (NV11_SetDenominationForChange(g_channels[channelIndex]))
                    {
                        return 1;
                    }
                    g_refundNote = g_channels[channelIndex];    /* Lay gia tri kenh tien te vao g_refundNote*/
                    break;
                }

                if (channelIndex > 7)
                {
                    break;
                }
            }


            /* 0x06 -> dau doc SMART Payout, 0x07 -> dau doc NV11 cho phep de dua tien len tren */
            if (setup_req.UnitType == 0x06 || setup_req.UnitType == 0x07)
            {
                /* Cho phep kich hoat tra lai tien -> ket qua ok = 0xF0 */
                if (ssp6_enable_payout(sspC, setup_req.UnitType) != SSP_RESPONSE_OK)
                {
                    Dbg_Println("NV11 >> Enable payout Failed");
                    g_countInitPayoutFail++;
                    sprintf(bufferDebug, "NV11 >> So lan khoi tao loi NV11 Payout %d", g_countInitPayoutFail);
                    Dbg_Println(bufferDebug);

                    if(g_countInitPayoutFail == 10)
                    {
                      g_countInitPayoutFail = 0;
                      g_unitType = setup_req.UnitType;
                      g_initialized = 1;
                      NV11_ClearLatestEvent();
                      NV11_DisplayOn();
                      g_initPayoutFault = true;
                      return 0;
                    }
                    return 1;
                }
                else
                {
                      g_countInitPayoutFail = 0;
                      g_unitType = setup_req.UnitType;
                      g_initialized = 1;
                      NV11_ClearLatestEvent();
                      NV11_DisplayOn();
                      return 0;
                }
            }
        }

        /* Neu dau doc tien la loai NV200 */
        else if (g_unitType == NV_TYPE_NV200)
        {
            for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
            {
                /* Lay cac kenh tien te cho phep tra lai cai dat trong ROM */
                if (Perh_GetRefundNote(channelIndex) == true)
                {
                    /* Cai dat menh gia tien te luu tru tren FloatNote de tra lai */
                    if (NV11_SetDenominationForChange(g_channels[channelIndex]))
                    {
                        return 1;
                    }
                    g_refundChannels[channelIndex] = true;
                }

                if (channelIndex > 7)
                {
                    break;
                }
            }
        }
    }

    g_initialized = true;       /* Xac nhan cai dat da hoan thanh */

    NV11_DisplayOn();           /* Cho phep dau doc hien thi khung vien */

    return 0;                   /* Bao trang thai da cai dat xong */
}
/**
 * @brief   Ham lay chung loai may doc tien
 *
 * @param   NONE
 * @retval  g_unitType -> Chung loai may doc tien
 */
int NV11_GetUnitType()
{
    return g_unitType;
}
/**
 * @brief   Ham cho phep thiet bi hoat dong, cai dat
 *
 * @param   NONE
 * @retval  int NV11_Enable -> Trang thai cai cho phep hoat dong (True or False)
 */
int NV11_Enable(void)
{
    if (!g_initialized)
    {
        return 1;
    }

    SSP_COMMAND* sspC = &g_sspCommand;

    /* enable the validator */
    if (ssp6_enable(sspC) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Enable NV11 Failed");
        return 1;
    }
//    Dbg_Println("NV11 >> Enable NV11 Successfully");
    return 0;
}
/**
 * @brief   Ham vo hieu hoa thiet bi
 *
 * @param   NONE
 * @retval  int NV11_Disable -> Trang thai vo hieu hoa (True or False)
 */
int NV11_Disable(void)
{
    if (!g_initialized)
    {
        return 1;
    }

    SSP_COMMAND* sspC = &g_sspCommand;

    /* disable the validator */
    if (ssp6_disable(sspC) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Disable Failed");
        return 1;
    }
    Dbg_Println("NV11 >> Disable Successfully");
    return 0;
}
/**
 * @brief   Ham cho phep thiet bi tra lai tien
 *
 * @param   NONE
 * @retval  int NV11_EnablePayoutb-> Trang thai cho phep tra lai tien (True or False)
 */
int NV11_EnablePayout(void)
{
    if (!g_initialized)
    {
        return 1;
    }

    /* Ham cho phep tra lai tien, setup_req.UnitType = NV11 */
    if (ssp6_enable_payout(&g_sspCommand, setup_req.UnitType) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Enable payout Failed");
//        g_latestEvent = NV11_ERROR;
        return 1;
    }
    Dbg_Println("NV11 >> Enable Payout Successfully");
    return 0;
}

/**
 * @brief   Ham cai dat cac menh gia tien chap nhan
 *
 * @param   NONE
 * @retval  0 neu tat chuc nang tra lai tien thanh cong
 *          1 neu ta chuc nang tra lai tien that bai
 */
bool NV11_SetAcceptNote(uint16_t channel) {
    uint8_t low = channel & 0xFF;
    uint8_t high = (channel >> 8) & 0xFF;
    if (ssp6_set_inhibits(&g_sspCommand, low, high) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Inhibits Failed");
        return 1;
    }
    Dbg_Println("NV11 >> Inhibits Successfully");
    return 0;
}

/**
 * @brief   Ham tat chuc nang tra lai tien
 *
 * @param   NONE
 * @retval  0 neu tat chuc nang tra lai tien thanh cong
 *          1 neu ta chuc nang tra lai tien that bai
 */
int NV11_DisablePayout(void)
{
    if (!g_initialized)
    {
        return 1;
    }

    /* Ham cho phep tra lai tien, setup_req.UnitType = NV11 */
    if (ssp6_disable_payout(&g_sspCommand) != SSP_RESPONSE_OK)
    {
        Dbg_Println("NV11 >> Disable payout Failed");
//        g_latestEvent = NV11_ERROR;
        return 1;
    }
    Dbg_Println("NV11 >> Disable Payout Successfully");
    return 0;
}
/**
 * @brief   Ham cai dat menh gia tien mat luu tru de tra lai
 *
 * @param   uint32_t denomination -> Menh gia tien cai dat (Vd: 1000, 2000, 5000, 10000, 20000, 50000)
 * @retval  int NV11_SetDenominationForChange -> Trang thai cai dat menh gia (True or False)
 */
int NV11_SetDenominationForChange(uint32_t denomination)
{
    /* Ham cai dat menh gia tien tra lai, gia tri = menh gia*100 (Vd: 5000*100) */
    if (ssp6_set_route(&g_sspCommand, denomination * 100, "VND", 0) != SSP_RESPONSE_OK)
    {
        sprintf(bufferDebug, "NV11 >> ERROR: Set routing failed: %d", denomination);
        Dbg_Println(bufferDebug);
        return 1;
    }
    sprintf(bufferDebug, "NV11 >> Set routing Successfully: %d", denomination);
    Dbg_Println(bufferDebug);
    return 0;
}
/**
 * @brief   Ham dinh tuyen menh gia tien tra lai cho NV200
 *
 * @param   g_refundChannels[channelIndex] -> Menh gia cai dat
 * @retval  int NV11_RoutingPayoutForNV200 -> Trang thai cai dat menh gia (True or False)
 */
int NV11_RoutingPayoutForNV200(void)
{
    int res = 0;
    for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
    {
        if (g_refundChannels[channelIndex] == true)
        {
            res = NV11_SetDenominationForChange(g_channels[channelIndex]);
            if (res > 0)
            {
                return res;
            }
        }
    }
    return 0;
}
/**
 * @brief   Ham cai dat tat ca cac menh gia tien mat luu tru de tra lai
 *
 * @param   g_channels[channelIndex] -> Tat ca cac menh gia tien cai dat (Vd: 1000, 2000, 5000, 10000, 20000, 50000)
 * @retval  int NV11_RoutingAllToCashbox -> Trang thai cai dat menh gia (True or False)
 */
int NV11_RoutingAllToCashbox(void)
{
    for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
    {
        if (ssp6_set_route(&g_sspCommand, g_channels[channelIndex] * 100, "VND", 1) != SSP_RESPONSE_OK)
        {
            Dbg_Println("NV11 >> ERROR: Set routing failed");
            return 1;
        }
        Dbg_Println("NV11 >> Set routing Successfully");
    }
    return 0;
}
/**
 * @brief  Ham thuc hien lenh giu tien (ngam tien) tai cua NV11
 *
 * @param  NONE
 * @retval int NV11_Hold -> Tra ve trang thai giu tien (OK -> F0, Fail -> 0xF8)
 */
int NV11_Hold(void)
{
    if (!g_initialized)                         /*Neu chua khoi tao, tra ket qua = 1*/
    {
        return 1;
    }

    g_isHolding = true;                         /* Bao dang giu (ngam) tien */
    return ssp6_hold(&g_sspCommand);
}
/**
 * @brief  Ham thuc hien lenh huy giu tien (ngam tien) tai cua NV11
 *
 * @param  NONE
 * @retval int NV11_UnHold -> Tra ve trang thai huy giu tien
 */
int NV11_UnHold(void)
{
    g_isHolding = false;                        /* Bao huy giu (ngam) tien */
    return 0;
}
/**
 * @brief  Ham tu choi nhan tien, tra lai tien cho khach
 *
 * @param  NONE
 * @retval int NV11_UnHold -> Tra ve trang thai huy giu tien
 */
int NV11_Reject(void)
{
    if (!g_initialized)                         /*Neu chua khoi tao, tra ket qua = 1*/
    {
        return 1;
    }

    g_isHolding = false;                        /* Bao huy giu (ngam) tien */
    ssp6_reject(&g_sspCommand);                 /* Thuc hien lenh tu choi nhan tien */
    while (g_latestEvent != NV11_NOTE_REJECTED) /* Kiem tra neu su kien cuoi khong phai la Reject */
    {
        NV11_Process();                         /* Thuc hien ham xu ly NV11 */

        /* Check tick all time */
        Perh_ProcessAllWhile();
    }
    return 0;                                   /* Thuc hien xong lenh tra ket qua = 0*/
}
/**
 * @brief  Ham reset bo dem luu tru ve 0, dieu huong cac khoan tien luu tru vao hop tien mat
 *
 * @param  NONE
 * @retval int NV11_UnHold -> Tra ve trang thai huy giu tien
 */
int NV11_EmptyNotes(void)
{
    if (!g_initialized)                 /*Neu chua khoi tao, tra ket qua = 1*/
    {
        return 1;
    }

    ssp6_empty_notes(&g_sspCommand);
    return 0;
}
/**
 * @brief  Ham lay gia tri so kenh tien te cua NV 11
 *
 * @param  NONE
 * @retval int int NV11_GetChannelNumbe -> Tra ve so kenh tien te
 */
int NV11_GetChannelNumber()
{
    return g_channelNumber;
}
/**
 * @brief  Ham lay menh gia tien trong tung kenh tien te cua NV11
 *
 * @param  NONE
 * @retval int int NV11_GetChannelNumbe -> Tra ve so kenh tien te
 */
int NV11_GetChannelValue(int channel)
{
    if (channel < g_channelNumber)              /* Neu so kenh can kiem tra < so kenh khai bao NV11 (6) */
    {
        return g_channels[channel];             /* Tra ket qua menh gia tien trong kenh */
    }
    return -1;                                  /* Neu lon hon, tra ket qua la -1*/
}
/**
 * @brief  NV200 Ham lay so to tien cua tung menh gia luu tru thanh toan (tra lai)
 *
 * @param  int channel -> Kenh menh gia can kiem tra
 * @retval int NV11_GetStoredNoteByChannel -> So to tien tuong ung voi kenh kiem tra
 */
int NV11_GetStoredNoteByChannel(int channel)
{
    if (channel < g_channelNumber)
    {
        return g_storedNotes[channel];
    }
    return -1;
}
/**
 * @brief  Ham lay menh gia luu tru thanh toan (tra lai) cua NV11
 *
 * @param  NONE
 * @retval int NV11_GetDenominationForChange -> Menh gia luu tru thanh toan
 */
int NV11_GetDenominationForChange(void)
{
    return g_refundNote;
}
/**
 * @brief  Ham tong so tien trong NoteFloat (thanh toan, tra lai)
 *
 * @param  NONE
 * @retval int NV11_GetAvailableChange -> Tong so tien co trong NoteFloat
 */
int NV11_GetAvailableChange(void)
{
    if (!g_initialized)         /*Neu chua khoi tao, tra ket qua = 1*/
    {
        return 0;
    }
    /* Neu la loai dau doc tien NV11 */
    if (g_unitType == NV_TYPE_NV11)
    {
        /*Tong so tien = so luong trong note Float * Menh gia tien tra lai
          NV11 cai dat duoc 1 menh gia tra lai
        */
        return (ssp6_get_stored_notes(&g_sspCommand) * NV11_GetDenominationForChange());
    }
    /* Neu la loai dau doc tien NV200 */
    else if (g_unitType == NV_TYPE_NV200)
    {
        int storedNotes = 0;
        for (int i = 0; i < g_channelNumber; i++)
        {
            /* Tong so tien = Tong (so luong menh gia x menh gia)
              NV 200 cai dat duoc nhieu menh gia tra lai
            */
            storedNotes += (g_storedNotes[i] * g_channels[i]);
        }
        return storedNotes;
    }
    /* Neu la loai dau doc tien khac, tra ket qua = 0 */
    else
    {
        return 0;
    }
}
/**
 * @brief  Ham kiem tra trang thai dau doc dang giu (ngam) tien hay khong
 *
 * @param  NONE
 * @retval g_latestNote -> Menh gia tien dang giu
 */
int NV11_IsHolding(void)
{
    if (!g_initialized)                         /* Neu chua khoi tao, tra ket qua = 1*/
    {
        return 1;
    }
    if (g_isHolding)
    {
        return g_latestNote;                    /* Tra ve menh gia tien dang giu */
    }
    return 0;
}

/**
 * @brief  Ham lay su kien gan nhat NV11 thuc hien
 *
 * @param  NONE
 * @retval g_latestEvent -> Su kien gan nhat
 */
nv11_event_t NV11_GetLatestEvent(void)
{
    return g_latestEvent;
}
/**
 * @brief  Ham xoa toan bo su kien
 *
 * @param  NONE
 * @retval NONE
 */
void NV11_ClearLatestEvent(void)
{
    g_latestEvent = NV11_EVENT_NONE;
}
/**
 * @brief  Ham lay menh gia to tien gan nhat NV11
 *
 * @param  NONE
 * @retval g_latestNote -> Menh gia to tien gan nhat
 */
int NV11_GetLatestNote(void)
{
    return g_latestNote;
}

/**
 * @brief  Ham xu ly dau doc tien
 *
 * @param  NONE
 * @retval g_latestEvent -> Cac su kien dau doc tien tra ve gan nhat
 */
void NV11_Process(void)
{
    SSP_RESPONSE_ENUM rsp_status;
    SSP_POLL_DATA6 poll;
    static uint8_t count = 0;

    if (!g_initialized)                         /* Neu chua khoi tao -> thoat*/
    {
        return;
    }

    if (GetTickCount() - g_oldTick > 100)
    {
        g_oldTick = GetTickCount();
        if (g_latestEvent == NV11_ERROR)        /* Neu su kien gan nhat bao NV11 loi */
        {
            count++;
            /* Check connection */
            if (count > 20)                     /* Neu su kien van giu trong 20 chu ki may*/
            {
                count = 0;
                Dbg_Println("NV11 >> NV11 is error. Trying to reconnect...");   /* Bao loi NV11 len man hinh debug*/
                g_resetNv11State = true;
                if (NV11_Init() == 0)                                           /* Thuc hien khoi tao lai NV11 */
                {
                    Dbg_Println("NV11 >> NV11 is working now");                 /* Neu ok -> bao NV11 hoat dong len man hinh debug*/
                    g_latestEvent = NV11_EVENT_NONE;                            /* Xoa su kien NV11 */
                }
            }
        }
        else if (g_isHolding == true)                                   /* Neu dau doc dang giu tien */
        {
            if (ssp6_hold(&g_sspCommand) != SSP_RESPONSE_OK)            /* Neu dat lenh giu tien Fail */
            {
                g_latestEvent = NV11_ERROR;                             /* Bao loi NV11 */
                g_isHolding = false;                                    /* Xoa trang thai dang giu tien */
            }
        }
        /* Cac su kien khac */
        else
        {
            /* Thuc hien ham doc cac su kien trong NV11, neu phan hoi ve khong phai la 0xF0 */
            if ((rsp_status = ssp6_poll(&g_sspCommand, &poll)) != SSP_RESPONSE_OK)
            {
                if (rsp_status == SSP_RESPONSE_TIMEOUT)                 /* Neu phan hoi su kien ve la SSP_RESPONSE_TIMEOUT */
                {
                    // If the poll timed out, then give up
                    Dbg_Println("NV11 >> SSP Poll Timeout");                    /* Bao len man hinh debug */
                    g_latestEvent = NV11_ERROR;                         /* Bao loi NV11 */
                }
                else
                {
                    if (rsp_status == 0xFA)                             /* Neu phai hoi su kien mot khoa ma hoa khong duoc cai dat*/
                    {
                        // The validator has responded with key not set, so we should try to negotiate one
                        if (ssp6_setup_encryption(&g_sspCommand,(unsigned long long)0x123456701234567LL) != SSP_RESPONSE_OK)    /* Thuc hien lai viec cai dat mot khoa ma hoa defaul */
                        {
                            Dbg_Println("NV11 >> Encryption Failed");   /* Cai dat loi, bao len man hinh debug */
                            g_latestEvent = NV11_ERROR;                 /* Bao loi NV11 */
                        }
                        else                                            /* Cai dat khoa ma hoa thanh cong */
                        {
                            Dbg_Println("NV11 >> Encryption Setup");    /* Bao hoan thanh cai dat */
                        }
                    }
                    else
                    {
                        g_latestEvent = NV11_ERROR;                     /* Cac phai hoi con lai, ma khong phai 0xF0 -> Bao loi NV11 */
                    }
                }
            }
            /* Thuc hien ham doc cac su kien trong NV11, neu phan hoi ve la 0xF0 -> phan tich cac su kien */
            else
            {
                parse_poll(&g_sspCommand, &poll);                       /* Thuc hien phan tich cac su kien NV11 */
            }
        }
    }
}


/* This is the blocking function */
/**
 * @brief  Ham thanh toan, tra lai tien dau doc NV11, moi lan thanh toan chi duoc 1 to = menh gia cai dat tra lai
 *
 * @param  NONE
 * @retval int NV11_PayoutNote -> Trang thai thanh toan (0 -> ok,1 -> Fail)
 */
int NV11_PayoutNote(void)
{
    uint8_t count_down_value = 60;

    if (!g_initialized)                                                 /* Neu chua khoi tao -> tra ket qua = 1 */
    {
        return 1;
    }
    Dbg_Println("NV11 >> Payout note start");
    SSP_RESPONSE_ENUM ret;
    uint32_t tick = 0;
    NV11_ClearLatestEvent();                                            /* Xoa toan bo su kien */
    g_ActionPayoutFault = false;
    /* FloatNote se thanh toan to tien cuoi cung vua duoc luu tru, neu co the tra tien ham se tra ra ket qua
      la 0xF0 = SSP_RESPONSE_OK*/
    ret = ssp6_payout_note(&g_sspCommand);                              /* Tra ve trang thai co the thanh toan hay khong */
    if (ret == SSP_RESPONSE_OK)                                         /* Neu co the thanh toan */
    {
        tick = GetTickCount();                                          /* Tinh time out */
        while (NV11_GetLatestEvent() != NV11_NOTE_DISPENSED)            /* Kiem tra co su kien tranh chap tien hay khong*/
        {
          /* hien thi dem nguoc */
            if((GetTickCount() - tick > 1000) && count_down_value != 0)
            {
              updateCountDownPayout(count_down_value);
              count_down_value--;
              tick = GetTickCount();
            }
            else if (count_down_value == 0)
            {
              Dbg_Println("NV11 >> Count down reject end");
              Dbg_Println("NV11 >> Note dispense error by timeout");
              g_ActionPayoutFault = true;
              return 1;
            }

            NV11_Process();                                             /* Thuc hien lenh xu ly cua NV11 */
            if(NV11_GetLatestEvent() == NV11_NOTE_DISPENSED)
            {
                Dbg_Println("NV11 >> Note dispense by flag");
                return 0;
            }

            if (NV11_GetLatestEvent() == NV11_ERROR)                    /* Neu co su kien NV11 loi -> thoat, tra gia tri = 1 */
            {
                Dbg_Println("NV11 >> Note dispense error by payout note");
                return 1;
            }

            /* Check tick all time */
            Perh_ProcessAllWhile();
        }
        return 0;                                                       /* Neu hoan thanh ok -> thoat, tra gia tri = 0*/
    }
    Dbg_Println("NV11 >> Note dispense error by nv11 error");
    g_latestEvent = NV11_ERROR;                                         /* Neu khong the thanh toan, nhan su kien NV11 loi */
    return 1;
}

bool NV11_GetStatusPayoutNote(void)
{
    return g_ActionPayoutFault;
}

/* This is the blocking function */
/**
 * @brief  NV200 Ham thanh toan, tra lai tien
 *
 * @param  NONE
 * @retval int NV11_PayoutValue -> Trang thai thanh toan (0 -> ok,1 -> Fail)
 */
int NV11_PayoutValue(int value)
{
    if (!g_initialized)                                                 /* Neu chua khoi tao -> tra ket qua = 1 */
    {
        return 1;
    }

    SSP_RESPONSE_ENUM ret;
    NV11_ClearLatestEvent();                                            /* Xoa toan bo su kien */
    /* Thuc hien lenh thanh toan, gia tri = menh gia*100, lenh 0x58 = Payout_amount, 0x19 = Test_Payout_amount*/
    ret = ssp6_payout(&g_sspCommand, value * 100, "VND", 0x58);
    if (ret == SSP_RESPONSE_OK)                                         /* Neu thuc hien thanh toan ok */
    {
        return 0;                                                        /* Thoat, tra gia tri = 0 */
    }
    return 1;                                                           /* Neu khong o -> Thoat, tra gia tri = 1 */
}

/* Ham su dung cho NV200 */
bool NV200_ValueIsOnForPayout(int value)
{
    for (int i = 0 ; i < g_channelNumber; i++)
    {
        if (g_channels[i] == value && g_refundChannels[i] == true)
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief  Ham bat led vien dau doc tien NV11
 *
 * @param  NONE
 * @retval int NV11_DisplayOn -> Trang thai hoan thanh
 */
int NV11_DisplayOn(void)
{
    if (!g_initialized)
    {
        return 1;
    }

    return ssp6_display_on(&g_sspCommand);
}
/**
 * @brief  Ham tat led vien dau doc tien NV11
 *
 * @param  NONE
 * @retval int NV11_DisplayOff -> Trang thai hoan thanh
 */
int NV11_DisplayOff(void)
{
    if (!g_initialized)
    {
        return 1;
    }

    return ssp6_display_off(&g_sspCommand);
}

/**
 * @brief  Ham phan tich cac su kien trong dau doc tien
 *
 * @param  NONE
 * @retval
 */
static void parse_poll(SSP_COMMAND *sspC, SSP_POLL_DATA6 * poll)
{
    int i;
    for (i = 0; i < poll->event_count; ++i)                                     /* Vong quet toan bo cac su kien tra ve co trong poll */
    {
        switch(poll->events[i].event)                                           /* Kiem tra su kien thu i*/
        {
        /* Su kien Reset -> Xay ra khi thiet bi duoc cap nguon và da tra qua qua trinh reset */
        case SSP_POLL_RESET:
            Dbg_Println("NV11 >> Unit Reset");
            /* Neu su kien nay xay ra, phai chac chan Vesion SSP dang su dung la V6 */
            if (ssp6_host_protocol(sspC, 0x06) != SSP_RESPONSE_OK)
            {
                Dbg_Println("NV11 >> Host Protocol Failed");                            /* Neu ko phai V6 -> Bao loi len man hinh Debug */
                return;
            }
            break;
        /* Su kien Read -> Xay ra khi dau doc dang doc tien giay */
        case SSP_POLL_READ:
            /* Neu xu kien doc co gia tri > 0 nghia la 1 to tien dang duoc doc va xac thuc */
            if (poll->events[i].data1 > 0)
            {
                /* In len man hinh kenh tien vua doc duoc (1 -> 6)*/
                sprintf(bufferDebug, "NV11 >> Note Read %ld %s", poll->events[i].data1, poll->events[i].cc);
                Dbg_Println(bufferDebug);

                if (poll->events[i].data1 > 0 && poll->events[i].data1 < 10)
                {
                    /* Bao su kien, trang thai may vua nhan tien -> NV11_NOTE_READ;*/
                    g_latestEvent = NV11_NOTE_READ;
                    /* Tra ve gia tri tien vua doc duoc */
                    g_latestNote = g_denominations[poll->events[i].data1 - 1];
                }
            }
            break;
        /* Su kien nay bao tien da duoc chuyen tu vi tri nhan tien sang vi tri ngan luu tru */
        case SSP_POLL_CREDIT:
            /* Dung dau doc tien */
            NV11_Disable();
            /* Tien da duoc chap nhan va chuyen vao ngan luu tru */
            sprintf(bufferDebug, "NV11 >> Credit %ld %s", poll->events[i].data1, poll->events[i].cc);
            /* Hien thi gia tri tien vua duoc luu tru (Kenh tien) */
            Dbg_Println(bufferDebug);
            break;
        /* Su kien nay say ra khi thiet bi mat dien luc dang trong qua trinh tra lai tien,
           gia tri tra lai -> so tien can phai thanh toan not */
        case SSP_POLL_INCOMPLETE_PAYOUT:
            /* In ra man hinh debug so tien con phai tra lai */
            sprintf(bufferDebug, "NV11 >> Incomplete payout %ld of %ld %s", poll->events[i].data1, poll->events[i].data2, poll->events[i].cc);
            Dbg_Println(bufferDebug);
            break;
        /* Su kien nay say ra khi thiet bi mat dien luc chua thuc hien viec tra lai
           gia tri tra lai -> So tien va su kien yeu cau*/
        case SSP_POLL_INCOMPLETE_FLOAT:
            /* In ra man hinh debug so tien va su kien yeu cau */
            sprintf(bufferDebug, "NV11 >> Incomplete float %ld of %ld %s", poll->events[i].data1, poll->events[i].data2, poll->events[i].cc);
            Dbg_Println(bufferDebug);
            break;
        /* Su kien dang trong qua trinh tra lai cho nguoi dung (vi tien bi tu choi) */
        case SSP_POLL_REJECTING:
            break;
        /* Su kien da hoan thanh viec tra lai tien cho nguoi tieu dung (vi tien bi tu choi) */
        case SSP_POLL_REJECTED:
            Dbg_Println("NV11 >> Note Rejected");
            g_latestEvent = NV11_NOTE_REJECTED;                 /* Nhan su kien tien bi tra lai */
            g_holdBalance = 0;
            g_isHolding = false;                                /* Xoa trang thai giu tien */
            break;
        /* Su kien thong bao tien dang duoc van chuyen den ngan xep */
        case SSP_POLL_STACKING:
            break;
        /* Su kien bao tien da duoc chuyen vao kho luu tru tra lai (NoteFloat) */
        case SSP_POLL_STORED:
            // The note has been stored in the payout unit
            Dbg_Println("NV11 >> Stored");
            /* Nhan su kien tien chuyen vao kho tra lai */
            g_latestEvent = NV11_NOTE_STORED;
            /* Neu la dau doc tien NV200 */
            if (g_unitType == NV_TYPE_NV200)
            {
                for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
                {
                    g_storedNotes[channelIndex] = ssp6_check_note_level(&g_sspCommand, g_channels[channelIndex]);
                }
            }
            /* Hien thi len man hinh debug tong so tien trong kho tra lai */
            g_numNotePayout = NV11_GetAvailableChange() / NV11_GetDenominationForChange();
            g_lastNumNotePayout = g_numNotePayout;
            sprintf(bufferDebug, "NV11 >> Stored Notes: %d", g_numNotePayout * NV11_GetDenominationForChange());
            Dbg_Println(bufferDebug);
            break;
        /* Su kien co mot to tien di tu tren float note xuong stack note */
        case SSP_POLL_NOTE_TRANSFERRED_TO_STACK:
            Dbg_Println("NV11 >> Note Transferred to Stacker");
            /* Thuc hien tra lai them 1 to tien */
            NV11_PayoutNote();
            break;
        /* Su kien bao tien da duoc chuyen vao ngan xep chong (kho khong tra lai) */
        case SSP_POLL_STACKED:
            /* Hien thi tien dang trong ngan xep */
            Dbg_Println("NV11 >> Stacked");
            /* Nhan su kien tien chuyen vao ngan xep */
            g_latestEvent = NV11_NOTE_STACKED;
            break;
        /* Khong co su kien nay */
        case SSP_POLL_SAFE_JAM:
            Dbg_Println("NV11 >> Safe Jam");
            break;
        /* Su kien to tien bi ket trong qua trinh van chuyen, gia tri tra ve vi tri bi ket */
        case SSP_POLL_UNSAFE_JAM:
            Dbg_Println("NV11 >> Unsafe Jam");
            break;
        /* Su kien tien bi ket */
        case SSP_POLL_JAMMED:
          sprintf(bufferDebug,"NV11 >> Jammed %ld", poll->events[i].data1);
          Dbg_Println(bufferDebug);
          break;
        /* Su kien Poll da bi vo hieu hoa */
        case SSP_POLL_DISABLED:

            //Dbg_Println("NV11 has been DISABLED");
            //NV11_Enable();
            break;
        /* Su kien phat hien mot no luc gian lan */
        case SSP_POLL_FRAUD_ATTEMPT:
            sprintf(bufferDebug, "NV11 >> Fraud Attempt %ld %s", poll->events[i].data1, poll->events[i].cc);
            Dbg_Println(bufferDebug);
            g_latestEvent = NV11_NOTE_FRAUD_ATTEMPT;
            break;
        /* Su kien bao ngan xep day */
        case SSP_POLL_STACKER_FULL:
            Dbg_Println("NV11 >> Stacker Full");
            break;
        /* Su kien bao hop tien bi thao */
        case SSP_POLL_CASH_BOX_REMOVED:
            Dbg_Println("NV11 >> Cashbox Removed");
            break;
        /* Su kien bao hop tien duoc thay the */
        case SSP_POLL_CASH_BOX_REPLACED:
            Dbg_Println("NV11 >> Cashbox Replaced");
            break;
        /* Su kien bao mot to tien da duoc dua vao luc thiet bi luc khoi dong, va da duoc dua ra khoi mat truoc cua may */
        case SSP_POLL_CLEARED_FROM_FRONT:
            sprintf(bufferDebug, "NV11 >> Cleared from front %ld",g_denominations[poll->events[i].data1 - 1]);
            Dbg_Println(bufferDebug);
            if (g_holdBalance != 0) g_holdBalance -= g_denominations[poll->events[i].data1 - 1];
            break;
        /* Su kien bao mot to tien da duoc dua vao luc thiet bi luc khoi dong, va da duoc dua ra khoi hop dung tien */
        case SSP_POLL_CLEARED_INTO_CASHBOX:
            sprintf(bufferDebug, "NV11 >> Cleared Into Cashbox %ld",g_denominations[poll->events[i].data1 - 1]);
            if (g_holdBalance != 0) g_holdBalance -= g_denominations[poll->events[i].data1 - 1];
            g_storedBalance += g_denominations[poll->events[i].data1 - 1];
            Dbg_Println(bufferDebug);
            break;
        /* Su kien tra ve tong gia tri ma thiet bi da phan chia de phan hoi lai lenh phan chia (Dispended command) */
        case SSP_POLL_DISPENSED:
            if (g_unitType == NV_TYPE_NV200)
            {
                for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
                {
                    g_storedNotes[channelIndex] = ssp6_check_note_level(&g_sspCommand, g_channels[channelIndex]);
                }
            }
            /* tinh toan so tien con lai co trong dau tra lai */
            Dbg_Println("NV11 >> Note dispensed");
            NV11_Enable();
            g_numNotePayout = NV11_GetAvailableChange() / NV11_GetDenominationForChange();
            sprintf(bufferDebug, "NV11 >> Stored Notes: %d", g_numNotePayout * NV11_GetDenominationForChange());
            Dbg_Println(bufferDebug);

            if(g_numNotePayout == 0)     /* neu khong co to tien nao trong dau tra lai thi kiem tra */
            {
                if(g_lastNumNotePayout > 1)   /* neu dot ngot tien tra lai ve khong */
                {
                    Dbg_Println("NV11 >> Loi tien tra lai ve 0");
                    g_errorPayout = true;
                    Perh_SetStateErrorPayout(g_errorPayout);
                }
                else if(g_lastNumNotePayout == 1) /* neu truoc do chi co 1 to tien thi cap nhat luong tien truoc do */
                {
                    g_lastNumNotePayout = g_numNotePayout;
                }
            }
            else if(g_numNotePayout != 0)     /* neu con tien trong dau tra lai thi cap nhat tien tra lai de so sanh ve sau */
            {
                g_lastNumNotePayout = g_numNotePayout;
            }
            g_latestEvent = NV11_NOTE_DISPENSED;
            break;
        /* Su kien bao thiet bi da hoan thanh viec lam trong (empty) de phan hoi lenh lam trong (Empty Command) */
        case SSP_POLL_EMPTIED:
            if (g_unitType == NV_TYPE_NV200)
            {
                for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
                {
                    g_storedNotes[channelIndex] = ssp6_check_note_level(&g_sspCommand, g_channels[channelIndex]);
                }
            }
            break;
        /* Su kien tra ve khi thiet bi hieu chuan phan cung that bai (loi phan cung) */
        case SSP_POLL_CALIBRATION_FAIL:
            /* Du lieu tra ve bao gom cac ma loi co san */
            Dbg_Println("NV11 >> Calibration fail: ");

            switch(poll->events[i].data1) {
                case NO_FAILUE:                                 /* Khong co loi*/
                    Dbg_Println ("NV11 >> No failure");
                case SENSOR_FLAP:                               /* Loi cam bien nap thanh toan */
                    Dbg_Println ("NV11 >> Optical sensor flap");
                case SENSOR_EXIT:                               /* Loi cam bien Exit */
                    Dbg_Println ("NV11 >> Optical sensor exit");
                case SENSOR_COIL1:                              /* Loi cam bien cuon 1 */
                    Dbg_Println ("NV11 >> Coil sensor 1");
                case SENSOR_COIL2:                              /* Loi cam bien cuon 2 */
                    Dbg_Println ("NV11 >> Coil sensor 2");
                case NOT_INITIALISED:                           /* Loi thiet bi khong duoc khoi tao */
                    Dbg_Println ("NV11 >> Unit not initialised");
                case CHECKSUM_ERROR:                            /* Loi checksum */
                    Dbg_Println ("NV11 >> Data checksum error");
                case COMMAND_RECAL:                             /* Hieu chinh lai bang lenh yeu cau (loi thoi)*/
                    Dbg_Println ("NV11 >> Recalibration by command required");
                    ssp6_run_calibration(sspC);
            }
            break;
        /* Su kien nay say ra khi MCU yeu cau tam dung thiet bi, chu trinh tra lai se tu dong huy -> bao loi qua trinh tra lai */
        /* Su dung cho dau doc NV200 */
        case 0xD6:  //Halted

            if (g_unitType == NV_TYPE_NV200)
            {
                Dbg_Println ("NV11 >> NV200 failed to pay the note out");
                for (int channelIndex = 0; channelIndex < g_channelNumber; channelIndex++)
                {
                    g_storedNotes[channelIndex] = ssp6_check_note_level(&g_sspCommand, g_channels[channelIndex]);
                }
            }
            sprintf(bufferDebug, "NV11 >> Stored Notes: %d", NV11_GetAvailableChange());
            Dbg_Println(bufferDebug);
            g_latestEvent = NV200_PAYOUT_FAILED;                /* Thiet lap su kien NV200 thanh toan loi */
            break;
        /* Doi voi cac su kien con lai, in ra ma hinh debug */
        default:
//            sprintf(str, "NV11 >> Poll Event: 0x%x", poll->events[i].event);
//            Dbg_Println(str);
            break;
        }
    }
}