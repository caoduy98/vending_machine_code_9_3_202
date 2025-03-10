
#include "platform.h"
#include "pin.h"

#define GPIO_INSTANCE_COUNT     5U

static GPIO_Type* g_gpioBase[] = GPIO_BASE_PTRS;
static PORT_Type* g_portBase[] = PORT_BASE_PTRS;
/**
 * @brief  Ham khoi tao kieu dau vao ra cho chan I/O
 *
 * @param  uint16_t pin -> Chan can khoi tao
 * @param  pin_mode_t mode -> Kieu dau vao ra (Input, Output, Input_pullup, Input_pulldown)
 * @retval NONE
 */
void Pin_Init(uint16_t pin, pin_mode_t mode)
{
    DEV_ASSERT((pin / 32) < GPIO_INSTANCE_COUNT);
    GPIO_Type* gpioBase = g_gpioBase[pin / 32];
    PORT_Type* portBase = g_portBase[pin / 32];
    uint8_t realPin = pin % 32;

    Pin_SetMux(pin, PIN_MUX_GPIO);

    if (mode == PIN_OUTPUT)
    {
        gpioBase->PDDR |= (1 << realPin);
    }
    else if (mode == PIN_INPUT)
    {
        gpioBase->PDDR &= ~(1 << realPin);
        portBase->PCR[realPin] &= ~PORT_PCR_PE_MASK;
    }
    else if (mode == PIN_INPUT_PULLUP)
    {
        gpioBase->PDDR &= ~(1 << realPin);
        portBase->PCR[realPin] |= PORT_PCR_PE_MASK;
        portBase->PCR[realPin] |= PORT_PCR_PS_MASK;
    }
    else if (mode == PIN_INPUT_PULLDOWN)
    {
        gpioBase->PDDR &= ~(1 << realPin);
        portBase->PCR[realPin] |= PORT_PCR_PE_MASK;
        portBase->PCR[realPin] &= ~PORT_PCR_PS_MASK;
    }
}
/**
 * @brief  Ham set dao kieu vao ra cho chan I/O
 *
 * @param  uint16_t pin -> Chan can dao kieu
 * @param  pin_mode_t mode -> Kieu dau vao ra (Input, Output)
 * @retval NONE
 */

void Pin_SetDir(uint16_t pin, pin_mode_t mode)
{
    DEV_ASSERT((pin / 32) < GPIO_INSTANCE_COUNT);
    GPIO_Type* gpioBase = g_gpioBase[pin / 32];
    uint8_t realPin = pin % 32;

    if (mode == PIN_OUTPUT)
    {
        gpioBase->PDDR |= (1 << realPin);
    }
    else if (mode == PIN_INPUT)
    {
        gpioBase->PDDR &= ~(1 << realPin);
    }
}
/**
 * @brief  Ham set chan theo Mux
 *
 * @param  uint16_t pin -> Chan can set
 * @param  pin_mux_t mux -> Lua chon Mux cho Pin
 * @retval NONE
 */
void Pin_SetMux(uint16_t pin, pin_mux_t mux)
{
    DEV_ASSERT(port < GPIO_INSTANCE_COUNT);
    uint8_t realPin = pin % 32;

    uint32_t tmp = g_portBase[pin / 32]->PCR[realPin];
    tmp &= ~PORT_PCR_MUX_MASK;
    tmp |= PORT_PCR_MUX(mux);
    g_portBase[pin / 32]->PCR[realPin] = tmp;
}

/**
 * @brief  Ham ghi du lieu dau ra cho chan output
 *
 * @param  uint16_t pin -> Chan ghi du lieu
 * @param  uint8_t value -> Du lieu can ghi (0 hoac 1)
 * @retval NONE
 */
void Pin_Write(uint16_t pin, uint8_t value)
{
    DEV_ASSERT((pin / 32) < GPIO_INSTANCE_COUNT);
    if (value == 0)
    {
        g_gpioBase[pin / 32]->PCOR |= (1 << (pin % 32));
    }
    else
    {
        g_gpioBase[pin / 32]->PSOR |= (1 << (pin % 32));
    }
}
/**
 * @brief  Ham dao trang thai chan dau ra
 *
 * @param  uint16_t pin -> Chan can dao trang thai
 * @retval NONE
 */
void Pin_Toggle(uint16_t pin)
{
    DEV_ASSERT((pin / 32) < GPIO_INSTANCE_COUNT);
    g_gpioBase[pin / 32]->PTOR |= (1 << (pin % 32));
}
/**
 * @brief  Ham ghi du lieu dau ra cho ca Port
 *
 * @param  uint16_t port -> Port can ghi du lieu
 * @param  uint32_t value -> Du lieu can ghi
 * @retval NONE
 */
void Pin_WritePort(uint16_t port, uint32_t value)
{
    DEV_ASSERT(port < GPIO_INSTANCE_COUNT);

    g_gpioBase[port]->PDOR = value;
}

/**
 * @brief  Ham doc du lieu dau vao cho chan Input
 *
 * @param  uint16_t pin -> Chan can doc du lieu
 * @retval uint8_t Pin_Read -> Du lieu doc duoc (0 hoac 1)
 */
uint8_t Pin_Read(uint16_t pin)
{
    DEV_ASSERT((pin / 32) < GPIO_INSTANCE_COUNT);
    if (g_gpioBase[pin / 32]->PDIR & (1 << (pin % 32U)))
    {
        return 1;
    }
    return 0;
}
/**
 * @brief  Ham doc du lieu dau vao cho ca Port
 *
 * @param  uint8_t port -> Port can doc du lieu
 * @retval uint32_t Pin_ReadPort -> Du lieu doc duoc (32 bit)
 */
uint32_t Pin_ReadPort(uint8_t port)
{
    DEV_ASSERT(port < GPIO_INSTANCE_COUNT);
    return g_gpioBase[port]->PDIR;
}


/* Configure the special function for pin */
void Pin_Config(uint16_t pin, pin_config_t* config)
{

}

