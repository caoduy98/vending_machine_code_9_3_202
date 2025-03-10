
#include "platform.h"
#include "keypad.h"
#include "tickcount.h"

#define KEY_HOLD_TIME1       1000
#define KEY_HOLD_TIME2       200
#define MAX_COUNT_STATE      3
#define STATE_CLICK          0
#define STATE_NONE           1
#define STATE_SCAN_PIN       STATE_CLICK
#define STATE_NONE_SCAN      STATE_NONE


static uint8_t g_keyPressed = 0;
static uint32_t g_oldTick = 0;
static key_callback_t g_callback[KEY_EVENT_NUM] = { NULL };

static char ScanKey();

/**
 * @brief   Ham khoi tao chan I/O cho ban phim
 * 
 * @param   NONE                     
 * @retval  NONE    
 */

void Keypad_Init(void)
{
    g_callback[0] = NULL;
    g_callback[1] = NULL;
    g_callback[2] = NULL;
    g_keyPressed = 0;

    Pin_Init(PIN_COL1, PIN_OUTPUT);
    Pin_Init(PIN_COL2, PIN_OUTPUT);
    Pin_Init(PIN_COL3, PIN_OUTPUT);
    Pin_Init(PIN_ROW1, PIN_INPUT_PULLUP);
    Pin_Init(PIN_ROW2, PIN_INPUT_PULLUP);
    Pin_Init(PIN_ROW3, PIN_INPUT_PULLUP);
    Pin_Init(PIN_ROW4, PIN_INPUT_PULLUP);

    Pin_Write(PIN_COL1, 1);
    Pin_Write(PIN_COL2, 1);
    Pin_Write(PIN_COL3, 1);
}

/**
 * @brief   Ham lay dia chi ham phuc vu su kien goi phim
 * 
 * @param   key_event_t event -> su kien goi phim 
 * @param   key_callback_t callback -> Dia chi ham tro toi de xu ly su kien                    
 * @retval  NONE    
 */

void Keypad_InstallCallback(key_event_t event, key_callback_t callback)
{
    if (event < KEY_EVENT_NUM)
    {
        g_callback[event] = callback;
    }
}


void Keypad_Process()
{
    char key = ScanKey();
    /* check if any key pressed or not */
    if (key != 0)   /* There is a key pressed */
    {
        if (g_keyPressed == 0)                                                  /* Kiem tra phim da duoc nhan truoc do hay chua */
        {
            g_keyPressed = key;                                                 /* Neu chua thi nhan vao xu ly */
            if (g_callback[KEY_PRESS] != NULL)                                  /* Kiem tra ham Keypad_InstallCallback Even KEY_PRESS co duoc goi hay khong */
            {
                g_callback[KEY_PRESS](KEY_PRESS, g_keyPressed);                 /* Neu duoc goi, thi truyen tham so vao ham de chay */
            }
            g_oldTick = GetTickCount();                                         /* Lay thoi gian tai thoi diem nhan phim */
        }
        else                                                                    /* Neu phim da duoc nhan truoc do */
        {
            if (GetTickCount() - g_oldTick > KEY_HOLD_TIME1)                    /* Kiem tra neu thoi gian nhan > KEY_HOLD_TIME1 */
            {
                g_oldTick = GetTickCount() - KEY_HOLD_TIME1 + KEY_HOLD_TIME2;   /* Giam thoi gian cho xuong cho vong kiem tra giu phim lan thu 2 tro di */
                /* Giam thoi gian check giu phim xuong = KEY_HOLD_TIME2*/
                if (g_callback[KEY_HOLD] != 0)                                  /* Kiem tra ham Keypad_InstallCallback Even KEY_HOLD co duoc goi hay khong */
                {
                    g_callback[KEY_HOLD](KEY_HOLD, g_keyPressed);               /* Neu duoc goi, thi truyen tham so vao ham de chay */
                }
            }
        }
    }
    else    /* There is no a key pressed */
    {
        if (g_keyPressed == 0)
            return;
        if (g_callback[KEY_RELEASE] != 0)                                       /* Kiem tra ham Keypad_InstallCallback Even KEY_RELEASE co duoc goi hay khong */                                      
        {
            g_callback[KEY_RELEASE](KEY_RELEASE, g_keyPressed);                 /* Neu duoc goi, thi truyen tham so vao ham de chay */
        }
        g_keyPressed = 0;                                                       
    }
}

/**
 * @brief   Ham doc trang thi tren 1 chan ban phim
 * 
 * @param   NONE                     
 * @retval  int trang thai chan doc ve cua ban phim 
 */
static uint8_t keypad_ReadPin(uint16_t pin)
{
    uint8_t count = 0;
    for(int i = 0; i < MAX_COUNT_STATE; i++)
    {
      if (Pin_Read(pin) == 0) count++;
      else count = 0;
      Delay(1);
    }
    if(count == MAX_COUNT_STATE) return STATE_CLICK;
    else return STATE_NONE;
}
/**
 * @brief   Ham quet ban phim ma tran
 * 
 * @param   NONE                     
 * @retval  char -> Ki tu ban phim   
 */
static char ScanKey()
{
    Pin_Write(PIN_COL1, STATE_SCAN_PIN);
    Pin_Write(PIN_COL2, STATE_NONE_SCAN);
    Pin_Write(PIN_COL3, STATE_NONE_SCAN);

    if (keypad_ReadPin(PIN_ROW1) == STATE_CLICK)
    {
        return '5';
    }
    else if (keypad_ReadPin(PIN_ROW2) == STATE_CLICK)
    {
        return '7';
    }
    else if (keypad_ReadPin(PIN_ROW3) == STATE_CLICK)
    {
        return '4';
    }
    else if (keypad_ReadPin(PIN_ROW4) == STATE_CLICK)
    {
        return '6';
    }

    Pin_Write(PIN_COL1, STATE_NONE_SCAN);
    Pin_Write(PIN_COL2, STATE_SCAN_PIN);
    Pin_Write(PIN_COL3, STATE_NONE_SCAN);

    if (keypad_ReadPin(PIN_ROW1) == STATE_CLICK)
    {
        return '1';
    }
    else if (keypad_ReadPin(PIN_ROW2) == STATE_CLICK)
    {
        return '3';
    }
    else if (keypad_ReadPin(PIN_ROW3) == STATE_CLICK)
    {
        return '0';
    }
    else if (keypad_ReadPin(PIN_ROW4) == STATE_CLICK)
    {
        return '2';
    }

    Pin_Write(PIN_COL1, STATE_NONE_SCAN);
    Pin_Write(PIN_COL2, STATE_NONE_SCAN);
    Pin_Write(PIN_COL3, STATE_SCAN_PIN);

    if (keypad_ReadPin(PIN_ROW1) == STATE_CLICK)
    {
        return '9';
    }
    else if (keypad_ReadPin(PIN_ROW2) == STATE_CLICK)
    {
        return '#';
    }
    else if (keypad_ReadPin(PIN_ROW3) == STATE_CLICK)
    {
        return '8';
    }
    else if (keypad_ReadPin(PIN_ROW4) == STATE_CLICK)
    {
        return '*';
    }

    return 0;
}
