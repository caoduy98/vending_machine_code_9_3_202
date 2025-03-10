/**
\file SFEMP3ShieldConfig.h

\brief Hardware dependent configuration definitions
\remarks comments are implemented with Doxygen Markdown format

This SFEMP3ShieldConfig.h helps configure the SFEMP3Shield library for
various supported different base Arduino boards and shield's using the
VS10xx chip. It is possible this may support other VS10xx chips. But are
unverified at this time.

As the name SFEMP3Shield implies this driver was originally developed from
Sparkfun's MP3 Player Shield. Whereas it can and has been easily adapted
to other hardware, both base Arduino's and shield's using the VS10xx.

The default configuration of this library assumes the SFE MP3 Shield, on
an UNO/Duemilanove, when left un-altered.

\note
Support forArduino Mega's REQUIRES additional jumpers. As the SPI are not on the same
pins as the UNO/Duemilanove.
When using a mega with SFE compatible shields jump the following pings :
<tt>
\n Mega's 51 to the MP3's D11 for MOSI
\n Mega's 50 to the MP3's D12 for MISO
\n Mega's 52 to the MP3's D13 for SCK
</tt>
\n The remainder of pins may remain unchanged. Including INT0 as the Mega maps
INT0 to D2 as to support USE_MP3_INTx, as is. Like the Uno.
\n Where the default SFEMP3ShieldConfig.h should not need changing.
\n Yes, SdFat's SoftSPI.h was tried, but has problems when used twice once with
Sd2Card.cpp and a 2nd time with SFEMP3Shield.cpp.

\n

\note
Support for Arduino Leonardo is afflicted by having the SPI pins not routing the same pins as the UNO. This is similar to the Arduino Mega. Where as it appears it should simply work with additional jumpers, from the Leonardo's ICSP port, which has the SPI pins to the MP3 shields equivalent SPI pins.
<tt>
\n Leo's ICSP4 to the MP3's D11 for MOSI
\n Leo's ICSP1 to the MP3's D12 for MISO
\n Leo's ICSP3 to the MP3's D13 for SCK
</tt>
\n and remember to \b NOT use D10 as an input. It must be left as output.

\todo Please let us know if this works? I think it should.

\sa SEEEDUINO as to how to configure for Seeeduino's Music Shield.
\sa GRAVITECH as to how to configure for Gravitech's MP3-4NANO Shield.
 */

#ifndef MP3Config_h
#define MP3Config_h

#include "pin_num.h"
#include <stdbool.h>
#include <stdint.h>



  #define _DREQ  GPIO_ReadPinInput(GPIO,MP3_DREQ_PORT,MP3_DREQ_PIN)

  #define XRESET_PIN
  #define XDCS_PIN
  #define XCS_PIN                 PB15
  #define DREQ_PỊN

  #define KCLOCK_XCS              kCLOCK_Gpio1
  #define MP3_XCS_PORT            1U 
  #define MP3_XCS_PIN             18U

  #define KCLOCK_XDCS             kCLOCK_Gpio1
  #define MP3_XDCS_PORT           1U    
  #define MP3_XDCS_PIN           17U 

  #define KCLOCK_DREQ             kCLOCK_Gpio2
  #define MP3_DREQ_PORT           2U       
  #define MP3_DREQ_PIN            2U    

  #define KCLOCK_RESET            kCLOCK_Gpio3
  #define MP3_RESET_PORT          3U  
  #define MP3_RESET_PIN           3U

 // #define SD_SEL              conffig in SPI_NXP.c


void config_pin();
void cs_high();
void cs_low();
void dcs_high();
void dcs_low();
void _reset(uint8_t res);


#endif