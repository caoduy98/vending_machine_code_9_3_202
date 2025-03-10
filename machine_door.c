/** @file    machine_door.c
  * @author
  * @version
  * @brief   Chuong trinh khoi tao, kiem tra trang thai dong mo cua
  */

#include "platform.h"
/* include debug */
#include "dbg_file_level.h"
#define DEBUG_LEVEL DEBUG_MACHINE_DOOR
#if DEBUG_LEVEL
#define Dbg_Println(x)
#define Dbg_Print(x)
#endif

#define DOOR_PIN    PB11

static uint8_t g_readCounter = 0;
static bool g_doorIsOpen = true;
static uint32_t g_tick = 0;
/**
 * @brief  Ham kiem tra trang thai cua
 *   
 * @param  NONE        
 * @retval bool Door_IsOpenInternal -> Trang thai cua (Neu True = cua mo, False = cua dong) 
 */
static bool Door_IsOpenInternal()
{
    if (Pin_Read(DOOR_PIN) == 0)                /* Neu dau vao cam bien cua = 0*/       
    {
        return true;                            /* Cua mo */
    }
    else
    {
        return false;                           /* Cua dong */                           
    }
}
/**
 * @brief  Khoi tao chan I/O cho cam bien cua
 *   
 * @param  NONE        
 * @retval NONE  
 */
void Door_Init()
{
    Pin_Init(DOOR_PIN, PIN_INPUT);              /* Cai dat la chan dau vao */
    g_doorIsOpen = Door_IsOpenInternal();       /* Nhan trang thai cua vao bie g_doorIsOpen */      
}

/**
 * @brief  Khoi tra ve trang thai cua mo
 *   
 * @param  NONE        
 * @retval bool Door_IsOpen -> Trang thai cua (True = cua mo, False = cua dong) 
 */
bool Door_IsOpen()
{
    return g_doorIsOpen;
}
/**
 * @brief  Khoi xu ly trang thai cua
 *   
 * @param  NONE        
 * @retval NONE
 */
void Door_Process()
{
    /* Kiem tra sau moi 10ms */
    if (GetTickCount() - g_tick > 10)
    {
        g_tick = GetTickCount();
        /* Neu co bao cua dang mo */
        if (g_doorIsOpen)
        {
            /* Neu cua da mo, kiem tra xem cua co dong hay khong */
            if (!Door_IsOpenInternal())
            {
                g_readCounter++;
                /* Cho sau 10 chu ki may */
                if (g_readCounter >= 10)
                {
                    g_readCounter = 0;
                    /* Xoa co bao cua dang mo (cua dong)*/
                    g_doorIsOpen = false;
                    Dbg_Println("Door >> Door is closed");
                }
            }
            else
            {
                g_readCounter = 0;
            }
        }
        /* Neu co bao cua dang dong */
        else
        {
            /* Neu cua da dong, kiem tra xem cua co dang mo hay khon g*/
            if (Door_IsOpenInternal())
            {
                g_readCounter++;
                /* Cho sau 10 chu ki may */
                if (g_readCounter >= 10)
                {
                    g_readCounter = 0;
                    /* Set co bao cua dang mo (cua mo) */
                    g_doorIsOpen = true;
                    Dbg_Println("Door >> Door is opened");
                }
            }
            else
            {
                g_readCounter = 0;
            }
        }
    }
}