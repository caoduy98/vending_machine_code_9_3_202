
#include "platform.h"
#include "buzzer.h"

#define BUZZER_FLASHING_FOREVER     0xFFFF

static int g_pin;
static timer_t g_tmrFlashing;
static int g_timeOn;
static int g_timeOff;
static int g_during;
static bool g_isOn = false;
static void TmrFlashing_Handler(timer_t* tmr, void* param);


void Buzz_Init(int pin)
{
    g_pin = pin;
    Pin_Init(g_pin, PIN_OUTPUT);
    Pin_Write(g_pin, 0);     /* Turn off the buffer */
}


void Buzz_On(void)
{
    Pin_Write(g_pin, 1);     /* Turn on the buffer */
}


void Buzz_Off(void)
{
    Pin_Write(g_pin, 0);     /* Turn off the buffer */
}


void Buzz_Flashing(int timeOn, int timeOff, int during)
{
    Buzz_On();
    g_isOn = true;
    g_during = during;
    g_timeOn = timeOn;
    g_timeOff = timeOff;
    Tmr_Start(&g_tmrFlashing, timeOn, TmrFlashing_Handler, NULL);
}


/* Timer handler */
static void TmrFlashing_Handler(timer_t* tmr, void* param)
{
    UNUSED(tmr);
    UNUSED(param);

    if (g_during > 0)
    {
        if (g_isOn == true)
        {
            Buzz_Off();
            if (g_during != BUZZER_FLASHING_FOREVER)
            {
                g_during--;
                if (g_during > 0)
                {
                    Tmr_Start(&g_tmrFlashing, g_timeOff, TmrFlashing_Handler, NULL);
                }
            }
        }
        else
        {
            Buzz_On();
            if (g_during != BUZZER_FLASHING_FOREVER)
            {
                g_during--;
                if (g_during > 0)
                {
                    Tmr_Start(&g_tmrFlashing, g_timeOn, TmrFlashing_Handler, NULL);
                }
            }
        }
    }
}

