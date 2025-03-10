
#include "platform.h"
#include "glcd.h"

#define LCD_DISPLAY_CLEAR                       0x01
#define LCD_FUNCTION_SET_BASIC_ALPHA            0x20
#define LCD_ENTRY_MODE_SET                      0x06
#define LCD_DISPLAY_ON                          0x0C
#define LCD_DISPLAY_OFF                         0x08
#define LCD_FUNCTION_SET_BASIC_GRAPHIC          0x22
#define LCD_FUNCTION_SET_EXTENDED_GRAPHIC       0x26
#define LCD_SET_GDRAM_ADDRESS                   0x80

extern const uint8_t g_vnfont_8x15[];
extern uint16_t g_vnfontFindPosition(uint16_t c);


static font_t* g_font;
static int g_dataPin = 0;
static int g_clockPin = 0;
static int g_csPin = 0;
static GPIO_Type* g_dataPort;
static GPIO_Type* g_clockPort;
static uint8_t g_dataPinInPort;
static uint8_t g_clockPinInPort;

static GPIO_Type* g_gpioBase[] = GPIO_BASE_PTRS;


static uint8_t g_offBuffer[(GLCD_WIDTH * GLCD_HEIGHT) / 8];

static void SendData(uint8_t data);
static void SendCommand(uint8_t command);
static void SendLcd(uint8_t data1, uint8_t data2);
static void SendSlow(uint8_t data);
static void CommandDelay();
static void GotoExtendedMode();
static void GotoBasicMode();
static void SetGraphicsAddress(unsigned int r, unsigned int c);

/**
 * @brief Ham cau hinh chan va khoi tao cho man hình LCD graphic 128x64
 * 
 * @param dataPin -> Chan truyen du lieu toi GLCD chuan noi tiep   
 * @param clockPin -> Chan clock  
 * @param csPin -> Chan cho phep hoat dong  
 * @retval NONE
 */

void GLcd_Init(int dataPin, int clockPin, int csPin)
{
    g_clockPin = clockPin;
    g_dataPin = dataPin;
    g_csPin = csPin;
    
    g_dataPort = g_gpioBase[dataPin / 32];
    g_clockPort = g_gpioBase[clockPin / 32];
    g_dataPinInPort = dataPin % 32;
    g_clockPinInPort = clockPin % 32;
    /* Cau hinh cac chan GLCD la dau ra */
    Pin_Init(g_clockPin, PIN_OUTPUT);                                                                                    
    Pin_Init(g_dataPin, PIN_OUTPUT);
    Pin_Init(g_csPin, PIN_OUTPUT);
    /* Dat muc dau ra ban dau cho cac chan GLCD*/
    Pin_Write(g_clockPin, 0);
    Pin_Write(g_dataPin, 0);
    Pin_Write(g_csPin, 1);

    SendCommand(LCD_FUNCTION_SET_BASIC_ALPHA);                                  /* Set dia chi DDRAM ve 0x00 va dua con tro ve goc */
    Delay(1);
    SendCommand(LCD_FUNCTION_SET_BASIC_ALPHA);                                  /* Set dia chi DDRAM ve 0x00 va dua con tro ve goc */
    Delay(1);
    SendCommand(LCD_ENTRY_MODE_SET);                                            /* Set vi tri con tro va hien thi dich chuyen khi doc ghi*/
    Delay(1);
    GotoBasicMode();                                                            /* Dua GLCD ve che do Basic */
    Delay(1);
    SendCommand(LCD_DISPLAY_CLEAR);                                             /* Xoa man hinh */
    Delay(50);
    GotoExtendedMode();                                                         /* Dua GLCD ve che do Extended */
    Delay(1);
    SendCommand(LCD_DISPLAY_ON);                                                /* Bat che do hien thi (diplay, cursor, blink) */                                                
    Delay(1);
    GLcd_ClearScreen(BLACK);                                                    /* Boi den man hinh trong mang g_offBuffer */                                                   
    GLcd_Flush();                                                               /* Day du lieu vao GDRAM */
    Delay(50);
    SendCommand(0x01);                                                          /* Xoa man hinh */
    Delay(50);

    GLcd_SetFont(&font6x8);                                                     /* Cai dat font 6x8 */
}


void GLcd_ClearScreen(int color)
{
    GLcd_FillRect(0, 0, GLCD_WIDTH, GLCD_HEIGHT, color);
}


void GLcd_SetFont(font_t* font)
{
    g_font = font;
}

/**
 * @brief   Ham ve 1 diem tren man hinh graphic (Thao tac bit tren 1 byte thuoc g_offBuffer)
 * 
 * @param   int x -> Toa do x diem hien thi 
 * @param   int y -> Toa do y diem hien thi
 * @param   int color -> Lua chon mau hien thi (theo define)          
 * @retval  
 */

void GLcd_DrawPoint(int x, int y, int color)
{
    if ((x >= 0) && (x < GLCD_WIDTH) && (y >= 0) && (y < GLCD_HEIGHT))
    {
        uint8_t *p = g_offBuffer + ((y * (GLCD_WIDTH / 8)) + (x / 8));          /*g_offBuffer Bo dem duoc su dung lu du lieu cua toan man hinh Graphic truoc khi truyen toi GLCD */
        uint8_t mask = 0x80u >> (x % 8);

        if (color == BLACK)
        {
            *p &= ~mask;
        }
        else
        {
            *p |= mask;
        }
    }
}
/**
 * @brief Ham ve mot ki tu ra man hinh           
 * 
 * @param char c -> Ki tu can hien thi  
 * @param int x -> Toa do x diem bat dau can hien thi   
 * @param int y -> Toa do y diem bat dau can hien thi          
 * @retval NONE  
 */
void GLcd_DrawChar(char c, int x, int y, int color)
{
    uint8_t i, j, k;
    /*Lay so byte tren mot hang, neu chia het cho 8 = ket qua phep chia, khong chia het 8 thi bang 1*/
    uint8_t bytesPerRow = g_font->width % 8 == 0? g_font->width / 8 : g_font->width / 8 + 1;  
    /* Bien template*/
    uint32_t tmp = 0;
    /*Lay byte dau tien cua ki tu can hien thi
      Vidu: Ki tu '1' = 0x31 (Hex) voi font 6x8, heght = 8, startCharacter = ' ' = 0x20, bytesPerRow = 1;
      p = (uint8_t*)(g_font->content) + 8*(0x31 - 0x20)*1;
      p = uint8_t*)(g_font->content) + 136 = byte dau tien cua trong mang cua ki tu '1' = 0x04;*/
    uint8_t* p = (uint8_t*)(g_font->content) + g_font->height * (c - g_font->startCharacter) * bytesPerRow;
    
    uint8_t tmpY = x + g_font->width - 1;                                       /* Diem cuoi cung ben phai cua mot hang, cach quet tu phai -> trai */                              

    for (i = 0; i < g_font->height; i++)                                        /* Quet toi toan bo chieu cao cua ki tu*/                
    {
        for (k = 0; k < bytesPerRow; k++)                                       /* Gui theo so byte cua ki tu tren 1 hang*/ 
        {
            tmp <<= 8;                                                          /* Dua tung byte trong mang vao tmp, toi da 4 byte*/
            tmp |= *(p + bytesPerRow - 1 - k);
        }
        p += bytesPerRow;
        for (j = 0; j < g_font->width; j++)                                     /* Quet de ve tung diem (dot) tren mot hang */
        {
            if (CHECKBIT(tmp, j))                                               /* Kiem tra tung bit trong tmp */
            {
                GLcd_DrawPoint(tmpY - j, y + i, color);                         /* Neu = 1 thi ve dot len man hinh*/         
            }
        }
    }
}
/**
 * @brief Ham ve mot ki tu tieng viet ra man hình(Unicode)<=> quet theo cot           
 * 
 * @param uint8_t* image -> Mang can hien thi 
 * @param int x -> Toa do x diem bat dau can hien thi   
 * @param int y -> Toa do y diem bat dau can hien thi 
 * @param uint8_t w -> Do rong cua anh can hien thi
 * @param uint8_t h -> Chieu cao cua anh can hien thi        
 * @retval NONE  
 */
void  GLcd_DrawCharUni(const uint8_t* image, int x, int y, uint8_t w, uint8_t h, int color)
{
    for (uint16_t i = 0; i < w; i++ )                                           /* Quet den het do rong ki tu (cot) */                                                    
    {
        for (uint16_t j = 0; j < h; j++)                                        /* Quet den het chieu cao ki tu (hang) */ 
        {
            if (READBIT(*(image + i + (j / 8) * w), j % 8))                     /* Lay byte dau tien ghep voi byte thu w trong trong mang cua ki tu */
            {                                                                   /* Kiem tra tung bit trong byte, neu bang 1 thi chuyen mau diem hien thi */
                GLcd_DrawPoint(x + i, y + j, color);
            }
        }
    }
}
/**
 * @brief Ham ve mot chuoi ki tu ra man hinh           
 * 
 * @param const char* str -> Chuoi ki tu can hien thi  
 * @param int x -> Toa do x diem bat dau can hien thi   
 * @param int y -> Toa do y diem bat dau can hien thi 
 * @param int color -> Mau hien thi         
 * @retval NONE  
 */
void GLcd_DrawString(const char* str, int x, int y, int color)
{
    while (*str != NULL)                                                        /* Quet den khi het ki tu */
    {
        GLcd_DrawChar(*str, x, y, color);                                       /* Xuat tung ki tu ra man hinh */
        if (*str == ' ')                                                        /* Neu ki tu la dau cach, bo qua 4 cot */
        {
            x = x + 4;
        }
        else                                                                    /* Neu la cac ki tu khac */
        {
            if (g_font->width > 20)                                             /* Neu do lon ki tu > 20 */
            {
                x += g_font->width + 3;                                         /* Toa do x cua ki tu tiep theo tang theo do rong ki tu + 3 */                                         
            }
            else                                                                /* Neu do lon ki tu < 20 */
            {
                x += g_font->width;                                             /* Toa do x cua ki tu tiep theo tang theo do rong ki tu */
            }
        }
        str++;                                                                  /* Chuyen toi ki tu tiep theo */
    }
}
/**
 * @brief Ham ve mot chuoi ki tu tieng viet ra man hình (Unicode)          
 * 
 * @param const uint16_t* s -> Mang can hien thi 
 * @param int x -> Toa do x diem bat dau can hien thi   
 * @param int y -> Toa do y diem bat dau can hien thi 
 * @param uint8_t w -> Do rong cua anh can hien thi
 * @param uint8_t h -> Chieu cao cua anh can hien thi        
 * @retval NONE  
 */
void GLcd_DrawStringUni(const uint16_t* s, int x, int y, int color)
{
    while (*s != 0)                                                             /* Quet den het ki tu */
    {
        if (*s != ' ')                                                          /* Neu khong phai la ki tu dau cach */
        {
            uint32_t charPosition = g_vnfontFindPosition(*s);                   /* Tra ve vi tri bat dau cua ki tu trong man Vnfont */
            
            /* Truyen byte thu 2 va byte thu w+1 cua ki tu vao ham ve ra man hinh */
            GLcd_DrawCharUni(&(g_vnfont_8x15[charPosition + 1]), x, y, g_vnfont_8x15[charPosition], 15, color);  
            /* Lay do lon w cua ki tu la byte dau tien */
            s++;                                                                /* Toi ki tu tiep theo */
            x += g_vnfont_8x15[charPosition] + 1;                               /* Toa do x ki tu tiep theo tang theo do lon ki tu w */
        }
        else                                                                    
        {
            s++;                                                                /* Neu la ki tu dau cach */
            x += 3;                                                             /* Bo qua 3 cot */
        }
    }
}

/**
 * @brief Ham kiem tra do lon cua chuoi ki tu         
 * 
 * @param const char* str -> Chuoi ki tu can kiem tra           
 * @retval int px -> Do lon cua chuoi ki tu can kiem tra  
 */
int  GLcd_MeasureString(const char* str)
{
    int px = 0;

    while (*str != NULL)                                                        /* Quet den het ki tu cua chuoi */
    {
        if (*str == ' ')                                                        /* Neu la ki tu dau cach, do lon chuoi tang them 4 */
        {
            px = px + 4;
        }
        else                                                                    /* Neu la ki tu khac */
        {
            if (g_font->width > 30)                                             /* Neu do lon ki tu > 30, do lon chuoi tang them = do lon ki tu + 3 */ 
            {
                px += g_font->width + 3;
            }
            else                                                                /* Neu do lon ki tu <= 30, do lon chuoi tang them = do lon ki tu */
            {
                px += g_font->width;
            }
        }
        str++;                                                                  /* Toi ki tu tiep theo */
    }

    return px;                                                                  /* Tra ket qua do lon chuoi sau khi het ki tu */
}

/**
 * @brief Ham kiem tra do lon cua chuoi ki tu (Voi ki tu tieng viet (Unicode)         
 * 
 * @param const uint16_t* str -> Chuoi ki tu can kiem tra           
 * @retval int px -> Do lon cua chuoi ki tu can kiem tra  
 */

int GLcd_MeasureStringUni(const uint16_t* str)
{
    int px = 0;
    while (*str != 0)
    {
        if (*str != ' ')
        {
            uint32_t charPosition = g_vnfontFindPosition(*str);
            px += g_vnfont_8x15[charPosition] + 1;
            str++;
        }
        else
        {
            str++;
            px += 3;
        }
    }
    return px;
}

/**
 * @brief Toa do vi tri giua cua chuoi ki tu         
 * 
 * @param const uint16_t* str -> Chuoi ki tu can kiem tra           
 * @retval int -> Toa do vi tri giua cua chuoi ki tu  
 */
int GLcd_CalculateXCenter(const uint16_t* s)
{
    return (GLCD_WIDTH - GLcd_MeasureStringUni(s)) / 2;
}

/**
 * @brief Ham ve mot duong thang len man hinh         
 * 
 * @param int x0 -> Toa do x diem bat dau  
 * @param int y0 -> Toa do y diem bat dau
 * @param int x1 -> Toa do x diem ket thuc
 * @param int y1 -> Toa do y diem ket thuc
 * @param int color -> Mau sac hien thi                
 * @retval NONE 
 */
void GLcd_DrawLine(int x0, int y0, int x1, int y1, int color)
{
    int deltaX = (x1 > x0)? (x1 - x0): (x0 - x1);
    int deltaY = (y1 > y0)? (y1 - y0): (y0 - y1);
    int stepX, stepY, error1, error2;

    stepX = (x0 < x1)? 1: (-1);
    stepY = (y0 < y1)? 1: (-1);

    error1 = deltaX - deltaY;
    while (1)
    {
        GLcd_DrawPoint(x0, y0, color);
        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }
        error2 = 2 * error1;
        if (error2 > -deltaY)
        {
            error1 = error1 - deltaY;
            x0 = x0 + stepX;
        }
        if (error2 < deltaX)
        {
            error1 = error1 + deltaX;
            y0 = y0 + stepY;
        }
    }
}

/**
 * @brief Ham ve mot hinh chu nhat len man hinh         
 * 
 * @param int x0 -> Toa do x diem bat dau  
 * @param int y0 -> Toa do y diem bat dau
 * @param int width -> Do lon hinh 
 * @param int height -> Chieu cao hinh
 * @param int color -> Mau sac hien thi                
 * @retval NONE 
 */
void GLcd_DrawRect(int x, int y, int width, int height, int color)
{
    uint8_t currentValue = 0;
    /* Draw the two horizontal lines */
    for (currentValue = 0; currentValue < width + 1; currentValue++)
    {
        GLcd_DrawPoint(x + currentValue, y, color);
        GLcd_DrawPoint(x + currentValue, y + height, color);
    }

    /* draw the two vertical lines */
    for (currentValue = 0; currentValue < height + 1; currentValue++)
    {
        GLcd_DrawPoint(x, y + currentValue, color);
        GLcd_DrawPoint(x + width, y + currentValue, color);
    }
}

/**
 * @brief Ham ve mot hinh tron len man hinh         
 * 
 * @param int x0 -> Toa do x diem bat dau  
 * @param int y0 -> Toa do y diem bat dau
 * @param int int radius -> Ban kinh hinh tron
 * @param int color -> Mau sac hien thi                
 * @retval NONE 
 */
void GLcd_DrawCircle(int x, int y, int radius, int color)
{
    int sw = 0, tmpY = 0, tmpX = 0;
    uint8_t d;

    d = y - x;
    tmpY = radius;
    sw = 3 - 2 * radius;
    while (tmpX <= tmpY)
    {
        GLcd_DrawPoint(x + tmpX, y + tmpY, color);
        GLcd_DrawPoint(x + tmpX, y - tmpY, color);

        GLcd_DrawPoint(x - tmpX, y + tmpY, color);
        GLcd_DrawPoint(x - tmpX, y - tmpY, color);

        GLcd_DrawPoint(y + tmpY - d, y + tmpX, color);
        GLcd_DrawPoint(y + tmpY - d, y - tmpX, color);
        GLcd_DrawPoint(y - tmpY - d, y + tmpX, color);
        GLcd_DrawPoint(y - tmpY - d, y - tmpX, color);

        if (sw < 0)
            sw += (4 * tmpX + 6);
        else
        {
            sw += (4 * (tmpX - tmpY) + 10);
            tmpY--;
        }

        tmpX++;
    }
}

/**
 * @brief Ham hien thi boi den mot hinh vuong hoac chu nhat      
 * 
 * @param int x0 -> Toa do x diem bat dau  
 * @param int y0 -> Toa do y diem bat dau
 * @param int width -> Do rong cua hinh
 * @param int height -> Chieu cao cua hinh
 * @param int color -> Mau sac cua hinh                 
 * @retval NONE 
 */
void GLcd_FillRect(int x, int y, int width, int height, int color)
{
    uint8_t i, j;
    for (i = x; i < x + width; i++)
        for (j = y; j < y + height; j++)
            GLcd_DrawPoint(i, j, color);
}
/**
 * @brief Ham ve mot anh len toan man hinh    
 * 
 * @param const uint8_t* img -> Mang chua anh can ve                 
 * @retval NONE 
 */

void GLcd_DrawImage(const uint8_t* img)
{
    int x = 0, y = 0, b = 0;
    int index = 0;

    for (y = 0; y < 8; y++)                                                     /* Quet lan luot 8 block, moi block la 128x8 */
    {
        for (x = 0; x < 128; x++)                                               /* Quet tong 128 cot */
        {
            for (b = 0; b < 8; b++)                                             /* Quet theo cot, moi cot 8 bit */
            {
                index = y * 128 + x;
                if (CHECKBIT(img[index], b))                                    /* Kiem tra neu bit bang 1 */
                {
                    GLcd_DrawPoint(x, y * 8 + b, WHITE);                        /* Ve diem tren man hinh mau trang */
                }
                else
                {
                    GLcd_DrawPoint(x, y * 8 + b, BLACK);                        /* Ve diem tren man hinh mau den */
                }
            }
        }
    }
}

/**
 * @brief Ham ve mot anh tai toa do bat ki   
 * 
 * @param const uint8_t *image -> Mang chua anh can ve 
 * @param int x -> Toa do x diem bat dau 
 * @param int y -> Toa do y diem bat dau                
 * @retval NONE 
 */

void GLcd_DrawBitmap(const uint8_t *image, int x, int y)
{
    int w = *image++;                                                           /* Lay do rong cua anh */
    int h = *image++;                                                           /* Lay chieu cao cua anh */
    
    for (uint16_t i = 0; i < w; i++ )
    {
        for (uint16_t j = 0; j < h; j++)
        {
            if (READBIT(*(image + i + (j / 8) * w), j % 8))
            {
                GLcd_DrawPoint(x + i, y + j, WHITE);
            }
        }
    }
}

/**
 * @brief Ham day du lieu vao GDRAM cua toan man hinh  
 * 
 * @param g_offBuffer -> Mang da duoc truyen du lieu tu cac ham phia tren 
 * @param GLCD_WIDTH -> Do lon man hinh = 128
 * @param GLCD_HEIGHT -> Chieu cao man hinh = 64             
 * @retval NONE 
 */

void GLcd_Flush(void)
{
    for (uint8_t r = 0; r < GLCD_HEIGHT; ++r)
    {
        uint8_t endColNum = (GLCD_WIDTH + 15)/16;                               /* +15 co y nghia gi */
        SetGraphicsAddress(r, 0);
        uint8_t *ptr = g_offBuffer + (16 * r);
        for (uint8_t i = 0; i < endColNum; ++i)
        {
            SendData(*ptr++);
            SendData(*ptr++);
        }
    }
}
/**
 * @brief Ham day du lieu vao GDRAM tu hang bat ky  
 * 
 * @param g_offBuffer -> Mang da duoc truyen du lieu tu cac ham phia tren 
 * @param int y -> Toa do cua hang bat ki
 * @param int height -> Chieu cao cua phan hien thi           
 * @retval NONE 
 */
void GLcd_FlushRegion(int y, int height)
{
    if (y < 0 || y >= GLCD_HEIGHT)
    {
        return;
    }
    
    for (uint8_t r = y; r < y + height; ++r)
    {
        uint8_t endColNum = (GLCD_WIDTH + 15)/16;
        SetGraphicsAddress(r, 0);
        uint8_t *ptr = g_offBuffer + (16 * r);
        for (uint8_t i = 0; i < endColNum; ++i)
        {
            SendData(*ptr++);
            SendData(*ptr++);
        }
    }
}


/***********PRIVATE FUNCTIONS*******************/

/**
 * @brief Ham day du lieu vao man hinh GLCD
 * 
 * @param uint8_t data -> Du lieu 8 bit can truyen     
 * @retval NONE 
 */
static void SendData(uint8_t data)
{
    SendLcd(0xFA, data);                                        /* Byte dau tien = 0xFA -> Dat lenh ghi toi RAM */                                                
}
/**
 * @brief Ham day command vao man hinh GLCD
 * 
 * @param uint8_t command -> Byte command 
 * @retval NONE 
 */
static void SendCommand(uint8_t command)
{
    SendLcd(0xF8, command);                                     /* Byte dau tien = 0xF8 -> Dat lenh ghi Command */ 
}

static void SendLcd(uint8_t data1, uint8_t data2)
{
    SendSlow(data1);                                            /* Byte dat lenh */                                             
    SendSlow(data2 & 0xF0);                                     /* Byte truyen 4 bit cao cua du lieu*/                                    
    SendSlow(data2 << 4);                                       /* Byte truyen 4 bit thap cua du lieu*/ 
}

static void SendSlow(uint8_t data)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        /* Set value to DATA Pin. Because the Pin_Write function takes time to execute, we have to 
           access directly to the registers to speed up the interface */
        if (data & 0x80)
        {
            g_dataPort->PSOR |= (1 << g_dataPinInPort);    // DATA Pin = 1
        }
        else
        {
            g_dataPort->PCOR |= (1 << g_dataPinInPort);    // DATA Pin = 0
        }
        
        /* Set CLOCK Pin to 1. Because the Pin_Write function takes time to execute, we have to 
           access directly to the registers to speed up the interface */
        g_clockPort->PSOR |= (1 << g_clockPinInPort);    
        for (int i = 0; i < 20; i++);
        g_clockPort->PCOR |= (1 << g_clockPinInPort);
        
        data <<= 1;
    }
}

static void CommandDelay()
{

}

static void GotoExtendedMode()
{
    SendCommand(LCD_FUNCTION_SET_EXTENDED_GRAPHIC);
    CommandDelay();
}

static void GotoBasicMode()
{
    SendCommand(LCD_FUNCTION_SET_BASIC_ALPHA);
    CommandDelay();
}
/**
 * @brief Set dia chi trong GDRAM
 * 
 * @param unsigned int r -> Dia chi hang 
 * @param unsigned int c -> Dia chi cot
 * @retval NONE 
 */

static void SetGraphicsAddress(unsigned int r, unsigned int c)
{
    SendCommand(LCD_SET_GDRAM_ADDRESS | (r & 31));
    CommandDelay();
    SendCommand(LCD_SET_GDRAM_ADDRESS | c | ((r & 32) >> 2));
    CommandDelay();
}
