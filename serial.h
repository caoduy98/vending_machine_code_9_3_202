
#ifndef SERIAL_H
#define SERIAL_H


#define SERIAL_PORT_NUM    3


typedef enum
{ 
    SERIAL_1_STOPBIT = 0,
    SERIAL_2_STOPBIT = 1
} serial_stopbit_t;


typedef enum
{ 
    SERIAL_PARITY_NONE = 0,
    SERIAL_PARITY_ODD = 1,
    SERIAL_PARITY_EVEN = 2
} serial_parity_t;

typedef struct serial_struct            serial_t;

typedef void (*serial_callbacl_t)(serial_t* obj, void* param);

typedef struct
{
    bool               useTxBuffer;
    bool               useRxBuffer;
    uint32_t           txHead;      /* The pointer that points to the head of tx buffer */
    uint32_t           txTail;      /* The pointer that points to the tail of tx buffer */
    uint32_t           rxHead;      /* The pointer that points to the head of rx buffer */
    uint32_t           rxTail;      /* The pointer that points to the tail of rx buffer */
    serial_callbacl_t  callback;
    void*              param;
} serial_private_t;

struct serial_struct
{
    uint8_t            port;
    uint32_t           baudrate;
    serial_stopbit_t   stopbit;
    serial_parity_t    parity;
    uint8_t*           txBuffer;
    uint32_t           txBufferSize;
    uint8_t*           rxBuffer;
    uint32_t           rxBufferSize;

    serial_private_t   privateObj;          /* This field is only used for internal driver */
};

void Serial_Init(serial_t* obj, uint32_t baudrate);
void Serial_SetBaudrate(serial_t* obj, uint32_t baudrate);
void Serial_WriteByte(serial_t* obj, uint8_t byte);
void Serial_Write(serial_t* obj, uint8_t* data, uint32_t length);

void Serial_InstallCallback(serial_t* obj, serial_callbacl_t callback, void* param);

uint32_t Serial_Available(serial_t* obj);
int      Serial_ReadByte(serial_t* obj);
int      Serial_PeekByte(serial_t* obj);

char* Serial_FindString(serial_t* obj, char* str);
void Serial_ClearRxBuffer(serial_t* obj);
void Serial_RxHandler(serial_t* obj, uint8_t rxData);


#endif  /* SERIAL_H */
