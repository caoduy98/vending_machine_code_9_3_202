#include "humidity.h"
#include "peripheral.h"
#include "math.h"
#include "pin.h"
#include "adc.h"
#include "tickcount.h"
#include "dbg_console.h"
#define clock 72U

#define CONTROL_HEADER_ON                       Pin_Write(DO1, 1);
#define CONTROL_HEADER_OFF                      Pin_Write(DO1, 0);

static float g_humidity = 0;
static float g_celsius = 0;

static uint16_t low_time;
static uint16_t high_time;
/**
 * @brief  Ham tao tre us
 *
 * @param  uint16_t us -> Thoi gian can tao tre (us)
 * @retval NONE
 */
void delayus1(uint16_t us)
{
    uint32_t ti = clock * us / 8;
    for( ;ti > 0; ti --);
}

/**
 * @brief  Ham kiem tra do am co duoi muc nguong duoi
 *
 * @param  float humidity - gia tri do am
 * @param  uint8_t thresholdLow - gia tri nguong duoi cua do am
 * @retval bool -  true neu do am o nguong duoi, false neu do am o tren nguong duoi
 */
static bool CheckThresholdLowTem(float humidity, uint8_t thresholdLow)
{
    static bool s_thresholdLowFlag;
    static uint32_t s_tick;
    if(humidity > thresholdLow)
    {
        s_thresholdLowFlag = false;
        s_tick = GetTickCount();
        return s_thresholdLowFlag;
    }
    else if(humidity < thresholdLow && GetTickCount() - s_tick > 10000)
    {
        s_thresholdLowFlag = true;
        return s_thresholdLowFlag;
    }
    return s_thresholdLowFlag;
}

/**
 * @brief  Ham kiem tra do am co tren muc nguong tren
 *
 * @param  float humidity - gia tri do am
 * @param  uint8_t thresholdHigh - gia tri nguong tren cua do am
 * @retval bool -  true neu do am o nguong duoi, false neu do am o tren nguong duoi
 */
static bool CheckThresholdHighTem(float humidity, uint8_t thresholdHigh)
{
    static bool s_thresholdHighFlag;
    static uint32_t s_tick;
    if(humidity < thresholdHigh)
    {
        s_thresholdHighFlag = false;
        s_tick = GetTickCount();
        return s_thresholdHighFlag;
    }
    else if(humidity > thresholdHigh && GetTickCount() - s_tick > 10000)
    {
        s_thresholdHighFlag = true;
        return s_thresholdHighFlag;
    }
    return s_thresholdHighFlag;
}


/**
 * @brief  Ham kiem tra do rong xung tu cam bien DHT
 *
 * @param  int state - gia tri logic can kiem tra do rong xung
 * @param  uint16_t timeout - gia tri toi da de check timout
 * @retval uint16_t - gia tri cycle con lai cua do rong xung - xung cang lon gia tri nay cang nho
 */
uint16_t pulseInLength(bool state, uint16_t timeout) {
    uint16_t cnt = timeout;
    while (Pin_Read(FX0) == state) {
        if (--cnt == 0)
            return 0;
    }
    return cnt;
}

/**
 * @brief  Ham doc 1 byte tu cam bien DHT
 *
 * @param  int state - gia tri logic can kiem tra do rong xung
 * @param  NONE
 */
uint8_t readByte(void) {
    uint8_t value = 0;
    for (uint8_t i = 0; i < 8; i++) {
        low_time = pulseInLength(0, 100);
        high_time = pulseInLength(1, 100);
        if( low_time> high_time) {
           value |= (1 << (7 - i));
        }
    }
    return value;
}

/**
 * @brief  Ham khoi tao i/o giao tiep voi cam ben DHT
 *
 * @param  NONE
 * @param  NONE
 */
void Humidity_Init() {
    Pin_Init(DO1, PIN_OUTPUT);
    CONTROL_HEADER_OFF
    Dbg_Println("Humidity_Init() [Done]");
}

/**
 * @brief  Ham doc gia tri do am va nhiet do cam ben DHT
 *
 * @param  float * pfValue - dia chi luu tru gia tri do am
 * @param  bool - true neu doc thanh cong - false neu doc that bai
 */
bool Read_Humidity(float * pfValue) {
    static uint32_t s_tick_read_sensor = 0;
    if((GetTickCount() - s_tick_read_sensor) < 3000) return false;
    s_tick_read_sensor = GetTickCount();

    uint8_t data[5];
    memset(data, 0x00 ,sizeof(data));

    // Start signal
    Pin_Init(FX0, PIN_OUTPUT);
    Pin_Write(FX0, 0);

    Delay(1);                   // Tbe: 0/8 1 20ms
    // Start read
    Pin_Init(FX0,PIN_INPUT_PULLUP);

    // Tgo: 20 80 200us
    high_time = pulseInLength(1, 100);
    if(!high_time) {
        return false;
    }

    // Trel 75 80 85us
    low_time = pulseInLength(0, 100);
    if(!low_time) {
        return false;
    }

    // Treh 75 80 85us
    high_time = pulseInLength(1, 100);
    if(!high_time) {
        return false;
    }

    for (int i = 0; i < 5; i++)
    {
        data[i] = readByte();
    }

    if (data[4] != (uint8_t)(data[0] + data[1] + data[2] + data[3])) {
        return false;
    }

    g_humidity = ((data[0] << 8) + data[1]) * 0.1;
    g_celsius = (((data[2] & 0x7F) << 8) + data[3]) * (data[2] & 0x80 ? -0.1 : 0.1);

    * pfValue = g_humidity;
    (void) (g_celsius);
    return true;
}

/**
 * @brief  Ham dieu khien so sanh do am, dieu khien On/Off
 *
 * @param  float humidity -> Nhiet do hien tai
 * @param  uint8_t humdset -> Nhiet do cai dat
 * @retval NONE
 */

/********************* H - 1 ************** H **************** H + 1 *************************/
/*********|  Headter OFF      |                                  | Headter ON ****************/
void Control_Humidity(float humidity,uint8_t humdset) {

    if(!Perh_HeaterIsOn()) return;

    static uint32_t s_time_ctrl = 0;
    static uint8_t s_first_run = 1;
    static uint8_t s_last_state_ctrl = 0xF; // 0: off , 1 on , other: nothing

    /*
     *   Kiem tra neu do am o duoi nguong, va da chay dien tro  > 1m thi co the tat
     */
    if(CheckThresholdLowTem(humidity, humdset - 1))
    {   if(s_last_state_ctrl != 0) {
            if(s_first_run) {
                s_first_run = 0;
                s_last_state_ctrl = 0;
                s_time_ctrl = GetTickCount();
                CONTROL_HEADER_OFF
            }
            else if((GetTickCount() - s_time_ctrl) > 60000){
                s_last_state_ctrl = 0;
                s_time_ctrl = GetTickCount();
                CONTROL_HEADER_OFF
            }
        }
        return;
    }

    /*
     *   Kiem tra neu do am o tren nguong, va da dun dien tro  > 5m thi co the bat
     */
    if(CheckThresholdHighTem(humidity, humdset + 1)) {
        if(s_last_state_ctrl != 1)  {
            if(s_first_run) {
                s_first_run = 0;
                s_last_state_ctrl = 1;
                s_time_ctrl = GetTickCount();
                CONTROL_HEADER_ON
            }
            else if((GetTickCount() - s_time_ctrl) > 300000){
                s_last_state_ctrl = 1;
                s_time_ctrl = GetTickCount();
                CONTROL_HEADER_ON
            }
        }


    }
}
