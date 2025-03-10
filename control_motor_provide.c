/** @file    setting_form_set_n1.c
  * @author
  * @version
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu cai dat thoi gian chay dong co cap huong
  */

/*********************************************************************************
 * INCLUDE
 */
#include "peripheral.h"
#include "motor.h"
#include "tickcount.h"
#include "gsm.h"
#include "nv11.h"

/*********************************************************************************
 * GLOBAL VARIABLE
 */
uint32_t g_n1_set = 0;
uint32_t g_n2_set = 0;
uint32_t g_n3_set = 0;
uint8_t s_state_ctrl = 0;
volatile uint32_t s_tick = 0;
uint32_t s_count_sell = 0;
uint32_t s_count_sell_when_empty = 0;
bool g_isStopSellWhenEmpty = false;
bool s_isStopSellWhenEmpty = false;
uint8_t g_is_half_stock = 0;
volatile uint32_t led_tick = 0;
extern bool isbanloi;

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Dem luong huong ban va dieu khien bang bien s_state_ctrl
 *
 * @param    uint32_t u32cnt : So luong huong da ban
 * @retval   NONE
 */
void CountSellAndCheckCtrlMotor(uint32_t u32cnt) {
    s_count_sell += u32cnt;
    if(s_count_sell < g_n2_set) {
        return;
    }
    s_count_sell = 0;
    s_state_ctrl = 0;
}
/**
 * @brief    Dem luong huong ban va dieu khien bang bien g_isStopSellWhenEmpty
 *
 * @param    uint32_t u32cnt : So luong huong da ban
 * @retval   NONE
 */
void CountSellAndCheckEmpty(uint32_t u32cnt) {
    s_count_sell_when_empty += u32cnt;
    if(s_count_sell_when_empty < g_n3_set) {
        g_isStopSellWhenEmpty = false;
        return;
    }
      g_isStopSellWhenEmpty = true;
      s_count_sell_when_empty = g_n3_set;
}

/**
 * @brief    Dem luong huong ban va dieu khien bang bien g_isStopSellWhenEmpty
 *
 * @param    uint32_t u32cnt : So luong huong da ban
 * @retval   NONE
 */
void ClearSellAndCheckEmpty(void) {
    g_isStopSellWhenEmpty = false;
    s_count_sell_when_empty = 0;
}

/**
 * @brief    Chuong trình dieu khien trang thai dong co
 *
 * @param    NONE
 * @retval   NONE
 */
void CtrlMotorProviderProcess(void) {
  if(isbanloi){ //isbanloi khi ban loi thi set len 1
        // Dieu khien nhap nhay
        if((GetTickCount() - led_tick) >= 500){
             Pin_Toggle(LED);
             led_tick = GetTickCount();
           }
        NV11_Disable(); //Them 2025_2_24
  }
  else{
       Pin_Write(LED, 1);
    }

    switch(s_state_ctrl) {
        case 0:
            MotorProvideRunSwap(); // Them 4/3/2025
            MotorProvideRun(); // run motor
            s_tick = GetTickCount();
            s_state_ctrl = 1;
            break;

        case 1:
            if((GetTickCount() - s_tick) >= (g_n1_set *1000) + 500) {
                MotorProvideStop(); // stop motor
                MotorProvideStopSwap(); // Them 4/3/2025
                s_state_ctrl = 2;
            }
            break;
            
        case 2: // Them 4/3/2025
            if((GetTickCount() - s_tick) >= (g_n1_set *1000) + 500) {
              
                MotorProvideStopSwap(); // Them 4/3/2025
                MotorProvideRun(); 
                s_state_ctrl = 3;
            }
            break;      
            
        case 3: // Them 4/3/2025
            if((GetTickCount() - s_tick) >= (g_n1_set *1000) + 3500) {
                MotorProvideStop(); 
                MotorProvideRunSwap(); // Them 4/3/2025
                s_state_ctrl = 4;
            }
            break;            
            
            
        default:
            break;      
      
    }

    if(s_isStopSellWhenEmpty != g_isStopSellWhenEmpty) {
        if(g_isStopSellWhenEmpty) {
            SendMotorErrorFrame(1);
        } else {
            SendMotorResumeFrame();
        }
        s_isStopSellWhenEmpty = g_isStopSellWhenEmpty;
    }

    //if(!SensorOurStock_IsEmpty()) {
        //ClearSellAndCheckEmpty();
    //}
  
       if(SensorOurStock_IsEmpty() && g_isStopSellWhenEmpty) { //24_2_2025
          isbanloi = 1; //Them 24_2_2025 
        }
}

/**
 * @brief    Chuong trình dieu khien trang thai dong co
 *
 * @param    NONE
 * @retval   NONE
 */
void CtrlMotorProviderProcessInterrupt(void) {
    switch(s_state_ctrl) {
        case 0:
            MotorProvideRunSwap(); // Them 4/3/2025
            MotorProvideRun(); // run motor
            s_tick = GetTickCount();
            s_state_ctrl = 1;
            break;

        case 1:
            if((GetTickCount() - s_tick) >= (g_n1_set *1000)) {
                MotorProvideStop(); // stop motor
                MotorProvideStopSwap(); // Them 4/3/2025
                s_state_ctrl = 2;
                
            }
            break;
            
        case 2: // Them 4/3/2025
            if((GetTickCount() - s_tick) >= (g_n1_set *1000) + 500) {  
                MotorProvideStopSwap(); // Them 4/3/2025
                MotorProvideRun(); 
                s_state_ctrl = 3;
            }
            break;      
            
        case 3: // Them 4/3/2025
            if((GetTickCount() - s_tick) >= (g_n1_set *1000) + 3500) {
                MotorProvideStop(); 
                MotorProvideRunSwap(); // Them 4/3/2025
                s_state_ctrl = 4;
            }
            break;             

        default:
            break;            
            
    }
    if(SensorOurStock_IsEmpty()) {
        g_is_half_stock = 1; 
    } else {
        g_is_half_stock = 0;
    } 
}