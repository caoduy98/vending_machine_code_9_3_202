
#include "platform.h"
#include "tickcount.h"


static uint32_t g_tick = 0;

void SysTick_Handler()
{
    g_tick++;
}

void Tick_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

void Delay(uint32_t ms)
{
    uint32_t currentTick = g_tick;
    while (g_tick - currentTick < ms);
}

uint32_t GetTickCount(void)
{
    return g_tick;
}


