#include "MP3Config.h"




void config_pin() 
{

gpio_pin_config_t output =
        {
        kGPIO_DigitalOutput, 0,
         };
gpio_pin_config_t input =
        {
        kGPIO_DigitalInput, 0,
         };



// conffig CS
 CLOCK_EnableClock(KCLOCK_DREQ); 

 GPIO_PinInit(GPIO,MP3_DREQ_PORT,MP3_DREQ_PIN, &input);

 GPIO_WritePinOutput(GPIO,MP3_DREQ_PORT,MP3_DREQ_PIN,1);




CLOCK_EnableClock(KCLOCK_XCS); 

 GPIO_PinInit(GPIO,MP3_XCS_PORT,MP3_XCS_PIN, &output);

 GPIO_WritePinOutput(GPIO,MP3_XCS_PORT,MP3_XCS_PIN,1);




 CLOCK_EnableClock(KCLOCK_XDCS); 

 GPIO_PinInit(GPIO,MP3_XDCS_PORT,MP3_XDCS_PIN, &output);

 GPIO_WritePinOutput(GPIO,MP3_XDCS_PORT,MP3_XDCS_PIN,1);



 CLOCK_EnableClock(KCLOCK_RESET); 

 GPIO_PinInit(GPIO,MP3_RESET_PORT ,MP3_RESET_PIN, &output);

 GPIO_WritePinOutput(GPIO,MP3_RESET_PORT ,MP3_RESET_PIN,1);



}


void cs_high()
{
   GPIO_WritePinOutput(GPIO,MP3_XCS_PORT,MP3_XCS_PIN,1);
}

void cs_low()
{
	GPIO_WritePinOutput(GPIO,MP3_XCS_PORT,MP3_XCS_PIN,0);
}

void dcs_high()
{
	GPIO_WritePinOutput(GPIO,MP3_XDCS_PORT,MP3_XDCS_PIN,1);
}

void dcs_low()
{
	GPIO_WritePinOutput(GPIO,MP3_XDCS_PORT,MP3_XDCS_PIN,0);
}


void _reset(uint8_t res)
{
	GPIO_WritePinOutput(GPIO,MP3_RESET_PORT ,MP3_RESET_PIN,res);
}


























