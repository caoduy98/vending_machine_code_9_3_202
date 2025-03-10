/** @file    watchdog.h
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

#ifndef __QRM_COMS_H_
#define __QRM_COMS_H_

/*********************************************************************************
 * INCLUDE
 */
#include "stdint.h"
#include "stdbool.h"

/*********************************************************************************
 * DEFINE
 */

/*********************************************************************************
 * TYPEDEFS
 */

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief  Init watch dogs
 * @param  None
 * @retval None
 */
void Wdog_Init(void);

/**
 * @brief  Refesh watchdog
 * @param  None
 * @retval None
 */
void Wdog_Refesh(void);

#endif /* __QRM_COMS_H_ */