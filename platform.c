
#include "platform.h"
#include "glcd.h"
#include "clock_config.h"
#include "serialfunc.h"
#include "ITLSSPProc.h"
#include "nv11.h"
#include "ff.h"
#include "eeprom.h"
#include "peripheral.h"
#include "motor.h"
#include "adc.h"
#include "humidity.h"
#include "language.h"
#include "swi2c.h"
#include "rtc.h"
#include "gsm.h"
#include "wav_player.h"
#include "watchdog.h"
#include "fsl_rcm.h"
 /* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_PLATFORM
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif

#define LCD_CS_PIN       PD14   /* RS */
#define LCD_DATA_PIN     PD15   /* RW */
#define LCD_CLK_PIN      PD13   /* E */
//#define LCD_CLK_PIN      PE9    /* E */
#define LCD_LED_PIN      PA9

static FATFS g_fatFs;
uint8_t buffer[20];
FIL file;
uint32_t br = 0;

swi2c_t g_softI2C0;
static volatile uint32_t StickreadTime = 0;
static RCM_Type *rcm_base = RCM;

int gHourtolog = 0;
int gminutetolog = 0;
int gSecondtolog = 0;
int gDatetolog = 0;
int gMonthtolog = 0;
int gYeartolog = 19;

extern bool g_initPayoutFault;
extern bool InitIsComplete;
extern uint16_t g_countInitPayoutFail;
extern void Door_Init();
extern void RestoreBalance();

static void CheckRebootReason()
{
    uint32_t stateRegister = RCM_GetPreviousResetSources(rcm_base);
    sprintf(bufferDebug, "RCM >> value reset %ld", stateRegister);
    Dbg_Println(bufferDebug);
    /*!< Watchdog reset */
    if ((stateRegister & kRCM_SourceWdog) == kRCM_SourceWdog)
    {
        Dbg_Println("Nguyen nhan reset la watch dog");
        RCM_ClearStickyResetSources(rcm_base, kRCM_SourceWdog);
    }
    /*!< External pin reset */
    else if ((stateRegister & 0x00000040) == 0x00000040)
    {
        Dbg_Println("Nguyen nhan reset la pin reset");
        RCM_ClearStickyResetSources(rcm_base, kRCM_SourcePin);
    }
    /*!< Power on reset */
    else if ((stateRegister & kRCM_SourcePor) == kRCM_SourcePor)
    {
        Dbg_Println("Nguyen nhan reset la power on reset");
        RCM_ClearStickyResetSources(rcm_base, kRCM_SourcePor);
    }
    /*!< Software reset */
    else if ((stateRegister & kRCM_SourceSw) == kRCM_SourceSw)
    {
        Dbg_Println("Nguyen nhan reset la software reset");
        RCM_ClearStickyResetSources(rcm_base, kRCM_SourceSw);
    }
    /*!< Low-voltage detect reset */
    else if ((stateRegister & kRCM_SourceLvd) == kRCM_SourceLvd)
    {
        Dbg_Println("Nguyen nhan reset la Low-voltage");
        RCM_ClearStickyResetSources(rcm_base, kRCM_SourceLvd);
    }
}

void PlatformInit(void)
{
    /* Initialize the clock at 48MHz */
    BOARD_BootClockRUN();                                       /* Khoi tao xung clock cho board*/

    CLOCK_EnableClock(kCLOCK_PortA);                            /* Khoi tao xung clock Port A */
    CLOCK_EnableClock(kCLOCK_PortB);                            /* Khoi tao xung clock Port B */
    CLOCK_EnableClock(kCLOCK_PortC);                            /* Khoi tao xung clock Port C */
    CLOCK_EnableClock(kCLOCK_PortD);                            /* Khoi tao xung clock Port D */
    CLOCK_EnableClock(kCLOCK_PortE);                            /* Khoi tao xung clock Port E */

    Tick_Init();                                                /* Khoi tao system Tick */
    srand(SysTick->VAL);

    Pin_Init(LCD_LED_PIN, PIN_OUTPUT);                          /* Khoi tao chan Led LCD la dau ra */
    Pin_Write(LCD_LED_PIN, 1);                                  /* Dat ghi tri mac dinh ban dau chan Led LCD = 1 */
    Keypad_Init();                                              /* Khoi tao I/O cho ban phim */
    GLcd_Init(LCD_DATA_PIN, LCD_CLK_PIN, LCD_CS_PIN);           /* Khoi tao I/O cho man hinh GLCD */

    /* Setup pins for NV11, setup UART1*/
    Pin_Init(PC8, PIN_INPUT_PULLUP);
    Pin_SetMux(PC8, PIN_MUX_ALT2);      /* LPUART1_RX */
    Pin_SetMux(PC9, PIN_MUX_ALT2);      /* LPUART1_TX */

    /* Setup pins for debug console, setup UART0 */
    Pin_SetMux(PB0, PIN_MUX_ALT2);      /* LPUART0_RX */
    Pin_SetMux(PB1, PIN_MUX_ALT2);      /* LPUART0_TX */

    g_softI2C0.sclPin = PD9;                                    /* Khai bao chan SCL cho I2C mem */
    g_softI2C0.sdaPin = PD8;                                    /* Khai bao chan SDA cho I2C mem */
    SWI2C_Init(&g_softI2C0);                                    /* Khoi tao I2C mem */
    DS1307_Init();
    Rtc_GetCurrentDate(&gDatetolog,&gMonthtolog,&gYeartolog);
    Rtc_GetCurrentTime(&gHourtolog,&gminutetolog,&gSecondtolog);

    Dbg_Init(0, 115200);                                        /* Khoi tao baudrate cho UART0*/
    Dbg_ClearScreen();                                          /* Xoa man hinh debug */
    Dbg_Println("Welcome to vending Machine");                  /* In chuoi ki tu len man hinh debug */

    Wdog_Init();
    CheckRebootReason();
    EE_Init();                                                  /* Khoi tao EEPROM */
    Perh_Init();

    Wdog_Refesh();
    if (f_mount(&g_fatFs, "", 0) != FR_OK)                      /* Kiem tra trang thai SDCARD */
    {
        GLcd_DrawString("SD CARD IS ERROR1", 0, 0, WHITE);
        GLcd_Flush();
    }
    else
    {
      FRESULT fr;
      FIL fil;
      UINT bw;
      fr = f_open(&fil,"check_sd.txt", FA_WRITE | FA_OPEN_ALWAYS);
      fr = f_lseek(&fil, f_size(&fil));
      if (fr == FR_OK)
      {
          fr = f_write(&fil,"Checksdcard\r\n",13,&bw);
          fr = f_write(&fil,"\n\r\n\r",4,&bw);
          fr = f_close(&fil);
          if( fr != FR_OK)
          {
            GLcd_DrawString("SD CARD IS ERROR", 0, 0, WHITE);
            GLcd_Flush();
          }
          else
          {
            GLcd_DrawString("SD CARD IS OK", 0, 0, WHITE);
            GLcd_Flush();
          }
      }
    }
    Wdog_Refesh();
    GLcd_DrawString("Init GSM/GPRS", 0, 10, WHITE);              /* Hien thi len man hinh GLCD trang thai khoi tao GSM/GPRS */
    GLcd_Flush();
    Dbg_Print("Init GSM/GPRS: ");
    Gsm_Init();                                                 /* Thuc hien viec khoi tao GSM/GPRS */
    GLcd_DrawString("Init GSM/GPRS: DONE", 0, 10, WHITE);
    GLcd_Flush();
    Dbg_Println("DONE");                                        /* Neu khoi tao thanh cong bao DONE -> Hien tai buoc nay dang bao ao */
    Wdog_Refesh();
    uint8_t data[7] = { 0 };
    EE_Read(SHOP_PASSWORD_EEP_ADDRESS, data, 6);                /* Doc password trong EEPROM*/
    /* Display password */
    Dbg_Print("Password for store owner: ");
    Dbg_Println(data);                                          /* In password ra man hinh debug */

    Dbg_Print("ADC Init:");
    ADC0_Init();                                                /* Khoi tao ADC0 */
    ADC1_Init();                                                /* Khoi tao ADC1 */
    Dbg_Println(" OK");
    Dbg_Print("Motor Init:");
    Motor_Init();                                               /* Khoi tao I/O dieu khien dong co */
    SensorOutStock_Init();
    Dbg_Println(" OK");
    Dbg_Print("Humidity Init:");
    Humidity_Init();                                           /* Khoi tao chan dieu khien nhiet do */
    Dbg_Println(" OK");
    Wdog_Refesh();

    Wav_Init();                                                 /* Khoi tao phat am thanh qua PWM */
    /* Initialize the NV11 */
    SetupSSPPort();                                             /* Khoi tao chuan giao tiep SSP voi dau doc tien*/
    ITLSSP_Init();

    Lang_RestoreFromMemory();                                   /* Lay ngon ngu cai dat duoc luu tren ROM*/

    if (DS1307_Init() == false)                                 /* Khoi tao RTC va kiem tra trang thai, neu loi hien thi len LCD */
    {
        GLcd_DrawString("Init RTC failed", 0, 40, WHITE);
        GLcd_Flush();
        Delay(500);
    }
    Wdog_Refesh();
    /* Initialize the NV11 */
    GLcd_DrawString("Init NV11", 0, 20, WHITE);
    GLcd_Flush();

    while (NV11_Init() != 0)                                    /* Khoi tao dau doc tien NV11, neu khong khoi tao duoc treo trong while */
    {
        Wdog_Refesh();
        Delay(1000);
    }


    Delay(500);
    if(g_initPayoutFault == false)
    {
        GLcd_DrawString("Init NV11: DONE", 0, 20, WHITE);           /* Neu khoi dong hoan thanh -> bao DONE */
    }
    else
    {
        GLcd_DrawString("Init NV11: Error POUT", 0, 20, WHITE);           /* Neu khoi dong hoan thanh -> bao DONE */
    }

    GLcd_Flush();
    Delay(2000);
    Wdog_Refesh();
    RestoreBalance();                                           /* Tra lai trang thai ngan chua tien */
    ReadEepromsalepointer();
    ReadEepromYearsalepointer();
    InitIsComplete = true;
    Wdog_Refesh();
}

void PlatformProcess(void)
{
    Tmr_Process();
    Keypad_Process();                                           /* Thuc hien xu ly ban phim */
    Gsm_Process();                                              /* Thuc hien xu ly giao tiep GSM */
    if(GetTickCount() > (StickreadTime + 5000))
    {
        StickreadTime = GetTickCount();
        Rtc_GetCurrentDate(&gDatetolog,&gMonthtolog,&gYeartolog);
        Rtc_GetCurrentTime(&gHourtolog,&gminutetolog,&gSecondtolog);
    }

    Perh_CheckDropSensorProcess();
}
