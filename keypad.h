
#ifndef KEYPAD_H
#define KEYPAD_H

//#define PIN_COL1    PD1
//#define PIN_COL2    PD7
//#define PIN_COL3    PB4
//#define PIN_ROW1    PB5
//#define PIN_ROW2    PE5
//#define PIN_ROW3    PE4
//#define PIN_ROW4    PA0

#define PIN_COL1    PE8
#define PIN_COL2    PB5
#define PIN_COL3    PC3
#define PIN_ROW1    PC2
#define PIN_ROW2    PD6
#define PIN_ROW3    PD7
#define PIN_ROW4    PD5



#define KEY_EVENT_NUM       3

typedef enum
{
    KEY_PRESS,
    KEY_HOLD,
    KEY_RELEASE,
} key_event_t;

typedef void (*key_callback_t)(key_event_t event, char code);                   /* Con tro ham, tro toi ham callback duoc goi trong Keypad_InstallCallback*/


void Keypad_Init(void);
void Keypad_InstallCallback(key_event_t event, key_callback_t callback);
void Keypad_Process(void);


#endif /* KEYPAD_H */
