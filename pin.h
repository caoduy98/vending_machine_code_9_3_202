
#ifndef PIN_H
#define PIN_H

#include "pin_num.h"
#include "pin_specific.h"

typedef enum
{
    PIN_OUTPUT,
    PIN_INPUT,
    PIN_INPUT_PULLUP,
    PIN_INPUT_PULLDOWN
} pin_mode_t;

typedef struct
{
    int tbd;
} pin_config_t;

void Pin_Init(uint16_t pin, pin_mode_t mode);
void Pin_SetDir(uint16_t pin, pin_mode_t mode);
void Pin_SetMux(uint16_t pin, pin_mux_t mux);

void Pin_Write(uint16_t pin, uint8_t value);
void Pin_WritePort(uint16_t port, uint32_t value);
void Pin_Toggle(uint16_t pin);

uint8_t  Pin_Read(uint16_t pin);
uint32_t Pin_ReadPort(uint8_t port);

/* Configure the special function for pin */
void Pin_Config(uint16_t pin, pin_config_t* config);

#endif  /* PIN_H */
