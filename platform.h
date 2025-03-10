
#ifndef PLATFORM_H
#define PLATFORM_H

#include "fsl_common.h"

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "tickcount.h"
#include "rtc.h"
#include "pin.h"
#include "serial.h"

#include "buzzer.h"
#include "serial_comm.h"

#include "dbg_console.h"
#include "sw_timer.h"

#include "keypad.h"
#include "resources.h"
#include "peripheral.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef UNUSED
#define UNUSED(x)   (void)x;
#endif

#ifndef DEV_ASSERT
#define DEV_ASSERT(condition)
#endif

/* Macro to set a bit in a register */
#ifndef SETBIT
#define SETBIT(addr, bit) 		((addr) |= (1 << (bit)))
#endif

/* Macro to clear a bit in a register */
#ifndef CLEARBIT
#define CLEARBIT(addr, bit)		((addr) &= ~(1 << (bit)))
#endif

/* Macro to check a bit in a register is 1 or 0 */
#ifndef CHECKBIT
#define CHECKBIT(addr, bit) 	(((addr) & (1 << (bit))) == (1 << (bit)))
#endif


#ifndef READBIT
#define READBIT(value, bit) (((value) >> (bit)) & 0x01)
#endif


#define DisableInterrupt()  __disable_irq()
#define EnableInterrupt()   __enable_irq()

void PlatformInit(void);
void PlatformProcess(void);

#endif  /* PLATFORM_H */
