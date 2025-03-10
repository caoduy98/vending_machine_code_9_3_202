
#ifndef PIN_PRIVATE_H
#define PIN_PRIVATE_H

typedef enum
{
    PIN_MUX_ANALOG = 0U,
    PIN_MUX_GPIO   = 1U,
    PIN_MUX_ALT2   = 2U,  /*!< chip-specific */
    PIN_MUX_ALT3   = 3U,  /*!< chip-specific */
    PIN_MUX_ALT4   = 4U,  /*!< chip-specific */
    PIN_MUX_ALT5   = 5U,  /*!< chip-specific */
    PIN_MUX_ALT6   = 6U,  /*!< chip-specific */
    PIN_MUX_ALT7   = 7U   /*!< chip-specific */
} pin_mux_t;

#endif  /* PIN_PRIVATE_H */
