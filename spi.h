
#ifndef SPI_WRAPPER_H
#define SPI_WRAPPER_H


#define SSPI_PORT_NUM    3


typedef enum
{
    SPI_MASTER,
    SPI_SLAVE
} spi_mode_t;


typedef enum
{
    SPI_LSBFIRST,
    SPI_MSBFIRST
} spi_bit_order_t;


typedef enum
{
    SPI_DATA_MODE0,      /* Clock Polarity (CPOL) = 0, Clock Phase (CPHA) = 0 */
    SPI_DATA_MODE1,      /* CPOL = 0, CPHA = 1 */
    SPI_DATA_MODE2,      /* CPOL = 1, CPHA = 0 */
    SPI_DATA_MODE3,      /* CPOL = 1, CPHA = 1 */
} spi_data_mode_t;


typedef struct
{
    uint8_t     port;
    spi_mode_t  mode;
} spi_t;


void Spi_Init(spi_t* obj, spi_mode_t mode);
void Spi_SetMode(spi_t* obj, spi_mode_t mode);
void Spi_SetSpeed(spi_t* obj, uint32_t speed);
void Spi_SetBitOrder(spi_t* obj, spi_bit_order_t bitOrder);
void Spi_SetDataMode(spi_t* obj, spi_data_mode_t mode);
void Spi_Transfer(spi_t* obj, uint8_t* txData, uint8_t* rxData, uint32_t size);
uint8_t Spi_TransferByte(spi_t* obj, uint8_t byte);



#endif  /* SPI_WRAPPER_H */
