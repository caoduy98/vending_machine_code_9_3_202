/** @file    watchdog.c
  * @version 1.0
  * @author  minhneo
  * @brief   
  *          + Initialization and de-initialization functions
  *             + 
  *          + Operation functions
  *             + 
  *          + Control functions
  *             + 
  *          + State functions
  *             + 
  *          + Callback function
  *             + 
  */

/*********************************************************************************
 * INCLUDE
 */
#include "watchdog.h"
#include "dbg_console.h"
#include "tickcount.h"

#include "fsl_wdog32.h"
#if defined(FSL_FEATURE_SOC_RCM_COUNT) && (FSL_FEATURE_SOC_RCM_COUNT)
#include "fsl_rcm.h"
#endif
#if defined(FSL_FEATURE_SOC_SMC_COUNT) && (FSL_FEATURE_SOC_SMC_COUNT > 1)
#include "fsl_msmc.h"
#endif

/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_WATCHDOG
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif
/*********************************************************************************
 * DEFINE
 */
#define EXAMPLE_WDOG_BASE WDOG
#define DELAY_TIME 100000U

#define WDOG_WCT_INSTRUCITON_COUNT (128U)

/*********************************************************************************
 * VARIABLE
 */
static WDOG_Type *wdog32_base = EXAMPLE_WDOG_BASE;
AT_QUICKACCESS_SECTION_DATA(static wdog32_config_t config);

/*********************************************************************************
 * STATIC FUNCTION INLINE
 */
static void WaitWctClose(WDOG_Type *base)
{
    /* Accessing register by bus clock */
    for (uint32_t i = 0; i < WDOG_WCT_INSTRUCITON_COUNT; i++)
    {
        (void)base->CNT;
    }
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/*!
 * @brief WDOG32 refresh testing
 *
 * Refresh WDOG32 in window and non-window mode.
 */

void Wdog_Init(void)
{   
    WaitWctClose(wdog32_base);
    
    WDOG32_GetDefaultConfig(&config);

    config.testMode = kWDOG32_UserModeEnabled;

    config.clockSource  = kWDOG32_ClockSource1;
    config.prescaler    = kWDOG32_ClockPrescalerDivide256;
    config.timeoutValue = 4500U; /* 7.5s */
    config.enableWindowMode = false;
    config.enableWdog32     = true;

    WDOG32_Init(wdog32_base, &config);
    Wdog_Refesh();
    
    Dbg_Println("Wdg >> Init watchdog success\r\n");
}

/**
 * @brief  Refesh watchdog
 * @param  None
 * @retval None
 */
void Wdog_Refesh(void)
{
  WDOG32_Refresh(wdog32_base);
}

