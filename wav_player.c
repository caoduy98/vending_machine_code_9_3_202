/** @file    wav_player.c
  * @author  
  * @version 
  * @brief   cung cap cac ham phuc vu phat nhac WAV trong the nho
  */

/*********************************************************************************
 * INCLUDE
 */

#include "wav_player.h"
#include "ff.h"
#include "fsl_acmp.h"
#include "fsl_lptmr.h"
#include "fsl_ftm.h"
#include "peripheral.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_WAV_PLAYER
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * MACRO
 */

#define USEC_TO_COUNT(us, clockFreqInHz) (uint64_t)((uint64_t)us * clockFreqInHz / 1000000U)

/*********************************************************************************
 * DEFINE
 */
#define WAV_BUFFER_SIZE     512
#define WAV_BUFFER_MASK     (WAV_BUFFER_SIZE - 1)


/*********************************************************************************
 * STATIC VARIABLE
 */
uint32_t tmpTick = 0;
uint16_t g_u8WaveIndexIn = 0, g_u8WaveIndexOut = 0;
static uint8_t g_buffer[2][WAV_BUFFER_SIZE];
static uint8_t g_playingBufferIndex = 0;
static bool g_bufferEmpty[2] = {true, true};
static bool g_playing = false;
static FIL g_file;
static int8_t g_volume = 0;

/*********************************************************************************
 * STATIC FUNCTION
 */
static void SetDacValue(uint8_t value);

/*********************************************************************************
 * GLOBAL FUNCTION
 */

/**
 * @brief    Ham xu ly ngat LPTimer0 de lay du lieu vao buffer
 * 
 * @param    NONE
 * @retval   NONE
 */
void PWT_LPTMR0_IRQHandler(void)
{
  /* clear interrupt flag*/
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
    /* lay du lieu vao trong buffer */
    Wav_Process();
}

/**
 * @brief    Ham xu ly ngat FTimer0 de phat nhac
 * 
 * @param    NONE
 * @retval   NONE
 */
void FTM0_IRQHandler()
{
    uint32_t statusFlags = FTM_GetStatusFlags(FTM0);

    if ((statusFlags & kFTM_TimeOverflowFlag) == kFTM_TimeOverflowFlag)
    {
      /* clear flag interrupt */
        FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);
       /* set gia tri vao time */
        SetDacValue(g_buffer[g_playingBufferIndex][g_u8WaveIndexOut]);
        ++g_u8WaveIndexOut;
        /* kiem tra xem da su dung gan het gia tri trong buffer chua*/
        if (g_u8WaveIndexOut >= WAV_BUFFER_MASK)
        {
            g_u8WaveIndexOut = 0;
            /* chuyen lay so lieu vao buffer da het */
            g_bufferEmpty[g_playingBufferIndex] = true;
            /* chuyen sang su dung bo buffer khac */
            g_playingBufferIndex = !g_playingBufferIndex;
        }
    }
}

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Xet PWM phat nhac
 * 
 * @param    uint8_t value   -  gia tri de xet PWM
 * @retval   NONE
 */
static void SetDacValue(uint8_t value)
{
    if (g_volume > 0)
    {
        FTM0->CONTROLS[kFTM_Chnl_2].CnV = value << g_volume;
    }
    else
    {
        FTM0->CONTROLS[kFTM_Chnl_2].CnV = value >> (g_volume * -1);
    }
    FTM0->SYNC |= FTM_SYNC_SWSYNC_MASK;
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Ham khoi tao ngat de phuc vu phat nhac
 * 
 * @param    NONE
 * @retval   NONE
 */
void Wav_Init()
{
    /* init low power timer */
    lptmr_config_t lptmrConfig;

    LPTMR_GetDefaultConfig(&lptmrConfig);
    LPTMR_Init(LPTMR0, &lptmrConfig);
    LPTMR_SetTimerPeriod(LPTMR0, USEC_TO_COUNT(10000, CLOCK_GetFreq(kCLOCK_LpoClk)));
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);

    Pin_SetMux(PD0, PIN_MUX_ALT2);

    ftm_config_t ftmInfo;
    ftm_chnl_pwm_signal_param_t ftmParam;

    /* Configure ftm params with frequency 24kHZ */
    ftmParam.chnlNumber = kFTM_Chnl_2;
    ftmParam.level = kFTM_HighTrue;
    ftmParam.dutyCyclePercent = 0;
    ftmParam.firstEdgeDelayPercent = 0U;

    FTM_GetDefaultConfig(&ftmInfo);
    ftmInfo.prescale = kFTM_Prescale_Divide_4;

    FTM_Init(FTM0, &ftmInfo);
    FTM_SetupPwm(FTM0, &ftmParam, 1U, kFTM_EdgeAlignedPwm, 16000U, CLOCK_GetFreq(kCLOCK_CoreSysClk));
    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
    NVIC_SetPriority(FTM0_IRQn, 1);     /* The priority of PWM must less than priority of drop sensor */
    EnableIRQ(FTM0_IRQn);

    g_u8WaveIndexIn = g_u8WaveIndexOut = 0;

    if (Perh_GetMediaVolume() > 10)
    {
        Wav_SetVolume(10);
    }
    else
    {
        Wav_SetVolume(Perh_GetMediaVolume());
    }

    NVIC_SetPriority(PWT_LPTMR0_IRQn, 2);   /* The priority of LPTMR must be less than priority of PWM */
    EnableIRQ(PWT_LPTMR0_IRQn);
}


/**
 * @brief    Phat nhac theo ten file co trong the nho
 * 
 * @param    const char* wav_file - ten file nhac trong the nho
 * @retval   NONE
 */
void Wav_Play(const char* wav_file)
{
    FRESULT fr;
    /* Neu khong cho php phat nhac thi thoat khoi chuong trinh */
    if (!Perh_AudioIsOn())
    {
        return;
    }
    /* Mo file nhac */
    Dbg_Println("Wav >> Opening wav file");
    fr = f_open(&g_file, wav_file, FA_OPEN_EXISTING | FA_READ);
    if (fr == FR_OK)
    {
        Dbg_Println("Wav >> Start Playing");
        /* Start timer to start playing */
        LPTMR_StartTimer(LPTMR0);
        FTM_StartTimer(FTM0, kFTM_SystemClock);
        g_playing = true;
    }
    else
    {
        Dbg_Println("Wav >> Can't read wav file");
        f_close(&g_file);
    }
}

/**
 * @brief     Dung phat nhac
 * 
 * @param    NONE
 * @retval   NONE
 */
void Wav_Stop()
{
    if (g_playing == false)
    {
        return;
    }

    FTM_StopTimer(FTM0);
    LPTMR_StopTimer(LPTMR0);
    f_close(&g_file);
    g_playing = false;
}


/**
 * @brief    Kiem tra trang thai dang phat nhac
 * 
 * @param    NONE
 * @retval   bool   true    -  dang phat nhac
 *                  false   -  khong phat nhac
 */
bool Wav_IsPlaying()
{
    return g_playing;
}

/**
 * @brief    Thiet lam am luong phat nhac
 * 
 * @param    uint8_t level - muc am luong
 * @retval   NONE
 */
void Wav_SetVolume(uint8_t level)
{
    if (level > 10)
    {
        level = 10;
    }

    g_volume = level - 8;
}

extern bool g_sdcardIsBusy;

/**
 * @brief    Lay du lieu file nhac vao trong bo dem
 * 
 * @param    NONE
 * @retval   NONE
 */
void Wav_Process()
{
    if (!g_playing)  return;
    UINT rb;
    uint8_t readingBuffer = !g_playingBufferIndex;

    if (g_bufferEmpty[readingBuffer])
    {
        if (g_sdcardIsBusy)
        {
            return;
        }
        f_read(&g_file, g_buffer[readingBuffer], WAV_BUFFER_SIZE, &rb);

        if (rb < WAV_BUFFER_SIZE)
        {
            /* We have reached to the end of file, so stop playing the audio here */
            FTM_StopTimer(FTM0);
            LPTMR_StopTimer(LPTMR0);
            f_close(&g_file);
            g_playing = false;
        }
        else
        {
            g_bufferEmpty[readingBuffer] = false;
        }
    }
}
