/* mbed VLSI VS1053b library
 * Copyright (c) 2010 Christian Schmiljun
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
/* This code based on:
 *  mbeduino_MP3_Shield_MP3Player
 *  http://mbed.org/users/xshige/programs/mbeduino_MP3_Shield_MP3Player/lgcx63
 *  2010-10-16
 */
 
#include "VS1053.h"
#include "MP3Config.h"
#include "SPI_NXP.h"


// patch binarys
#include "Patches/VS1053b_patch_1_5.c"
#include "Patches/VS1053b_patch_1_5_flac.c"
#include "Patches/VS1053b_patch_1_4_flac.c"
#include "Patches/VS1053b_specana_0_9.c"
#include "Patches/VS1053b_pcm_recorder_0_9.c"



                
       uint8_t*                        buffer ;
       uint16_t                          buffer_size;

       uint8_t*                            _buffer;
       uint8_t*                           _bufferReadPointer;
       uint8_t*                           _bufferWritePointer;
       uint16_t                             BUFFER_SIZE;
    
        bool                            _isIdle;

          // variables to save 
          //   volume, values in db
        float                           _balance;    
        float                           _volume;
          //   bass enhancer settings  
        int                             _sb_amplitude;
        int                             _sb_freqlimit;
        int                             _st_amplitude;
        int                             _st_freqlimit; 
       
       
    
               // _sampleRateTable[id][srate]
    

const uint8_t _sampleRateTable[4][4] = {
    11, 12, 8, 0,
    11, 12, 8, 0,
    22, 24, 16, 0,
    44, 48, 32, 0
};

/* ==================================================================
 * Constructor
 * =================================================================*/
void VS1053(
   uint8_t* buffer, uint16_t buffer_size)
        
        
{           
        _volume = DEFAULT_VOLUME;
        _balance = DEFAULT_BALANCE_DIFERENCE_LEFT_RIGHT;
        _sb_amplitude = DEFAULT_BASS_AMPLITUDE;
        _sb_freqlimit = DEFAULT_BASS_FREQUENCY;
        _st_amplitude = DEFAULT_TREBLE_AMPLITUDE;
        _st_freqlimit = DEFAULT_TREBLE_FREQUENCY;   
        _buffer = buffer;
        BUFFER_SIZE = buffer_size;
      //  _DREQ_INTERUPT_IN.mode(PullDown);
      //  INTERRUPT_HANDLER_DISABLE;
        bufferReset();
}


/*===================================================================
 * Functions
 *==================================================================*/

void sci_en(void) {                  //SCI enable
    cs_high();
    dcs_high();
    cs_low();
}
void sci_dis(void) {                  //SCI disable
    cs_high();
}
void sdi_en(void) {                  //SDI enable
    dcs_high();
    cs_high();
    dcs_low();
}
void sdi_dis(void) {                  //SDI disable
    dcs_high();
}
void reset(void) {                  //hardware reset
   // INTERRUPT_HANDLER_DISABLE;
    delayms(10);
    _reset(0);
    delayms(5);
    _reset(1);
    delayms(10);
}
void power_down(void) {              //hardware and software reset
    cs_low();
    reset();
//    sci_write(0x00, SM_PDOWN);
    sci_write(0x00, 0x10); // tempo
    delayms(1);
    reset();
}
void spi_initialise(void) {
    _reset(1);                                //no reset
    //_spi.format(8,0);                        //spi 8bit interface, steady state low
//   _spi.frequency(1000000);                //rising edge data record, freq. 1Mhz
    //_spi.frequency(2000000);                //rising edge data record, freq. 2Mhz


    cs_low();
    for (int i=0; i<4; i++) {
        SPI_tbyte(0xFF);                        //clock the chip a bit
    }
    cs_high();
    dcs_high();
    delayus(5);
}
void sdi_initialise(void) {
                  //set to 12 MHz to make fast transfer
    cs_high();
    dcs_high();
}
void sci_write(unsigned char address, unsigned short int data) {
    // TODO disable all interrupts
  //  __disable_irq();
    sci_en();                                //enables SCI/disables SDI

    while (!_DREQ);                           //wait unitl data request is high
    SPI_tbyte(0x02);                        //SCI write
    SPI_tbyte(address);                    //register address
    SPI_tbyte((data >> 8) & 0xFF);            //write out first half of data word
    SPI_tbyte(data & 0xFF);                //write out second half of data word

    sci_dis();                                //enables SDI/disables SCI
    delayus(5);
    
    // TODO enable all interrupts
  //  __enable_irq();
}
void sdi_write(unsigned char datum) {    
    
    sdi_en();

    while (!_DREQ);
    SPI_tbyte(datum);

    sdi_dis();    
}
unsigned short sci_read(unsigned short int address) {
    // TODO disable all interrupts
  //  __disable_irq();
    
    cs_low();                                //enables SCI/disables SDI

    while (!_DREQ);                           //wait unitl data request is high
    SPI_tbyte(0x03);                        //SCI write
    SPI_tbyte(address);                    //register address
    unsigned short int received = SPI_rbyte(0x00);    //write out dummy byte
    received <<= 8;
    received |= SPI_rbyte(0x00);            //write out dummy byte

    cs_high();                                //enables SDI/disables SCI

    // TODO enable all interrupts
  //  __enable_irq();
    return received;                        //return received word
}
void sine_test_activate(unsigned char wave) {
    cs_high();                                //enables SDI/disables SCI

    while (!_DREQ);                           //wait unitl data request is high
    SPI_tbyte(0x53);                        //SDI write
    SPI_tbyte(0xEF);                        //SDI write
    SPI_tbyte(0x6E);                        //SDI write
    SPI_tbyte(wave);                        //SDI write
    SPI_tbyte(0x00);                        //filler byte
    SPI_tbyte(0x00);                        //filler byte
    SPI_tbyte(0x00);                        //filler byte
    SPI_tbyte(0x00);                        //filler byte

    cs_low();                                //enables SCI/disables SDI
}
void sine_test_deactivate(void) {
    cs_high();

    while (!_DREQ);
    SPI_tbyte(0x45);                        //SDI write
    SPI_tbyte(0x78);                        //SDI write
    SPI_tbyte(0x69);                        //SDI write
    SPI_tbyte(0x74);                        //SDI write
    SPI_tbyte(0x00);                        //filler byte
    SPI_tbyte(0x00);                        //filler byte
    SPI_tbyte(0x00);                        //filler byte
    SPI_tbyte(0x00);                        //filler byte
}

unsigned short int wram_read(unsigned short int address) {
    unsigned short int tmp1,tmp2;
    sci_write(SCI_WRAMADDR,address);
    tmp1=sci_read(SCI_WRAM);
    sci_write(SCI_WRAMADDR,address);
    tmp2=sci_read(SCI_WRAM);
    if (tmp1==tmp2) return tmp1;
    sci_write(SCI_WRAMADDR,address);
    tmp1=sci_read(SCI_WRAM);
    if (tmp1==tmp2) return tmp1;
    sci_write(SCI_WRAMADDR,address);
    tmp1=sci_read(SCI_WRAM);
    if (tmp1==tmp2) return tmp1;
    return tmp1;
}

void wram_write(unsigned short int address, unsigned short int data) {
    sci_write(SCI_WRAMADDR,address);
    sci_write(SCI_WRAM,data);
    return;
}

void setPlaySpeed(unsigned short speed)
{
    wram_write(para_playSpeed, speed);
    if(DEBUG) PRINTF("VS1053b: Change speed. New speed: %d\r\n", speed);
}

void terminateStream(void) {
    while(bufferCount() > 0) 
        ;
    if(DEBUG) PRINTF("VS1053b: Song terminating..\r\n");
    // send at least 2052 bytes of endFillByte[7:0].
    // read endFillByte  (0 .. 15) from wram 
    unsigned short endFillByte=wram_read(para_endFillByte);
    // clear endFillByte (8 .. 15)
    endFillByte = endFillByte ^0x00FF;                          
    for (int n = 0; n < 2052; n++) 
        sdi_write(endFillByte);
    
    // set SCI MODE bit SM CANCEL    
    unsigned short sciModeByte = sci_read(SCI_MODE);        
    sciModeByte |= SM_CANCEL;    
    sci_write(SCI_MODE, sciModeByte);
           
    // send up 2048 bytes of endFillByte[7:0]. 
    for (int i = 0; i < 64; i++) 
    { 
        // send at least 32 bytes of endFillByte[7:0]
        for (int n = 0; n < 32; n++) 
            sdi_write(endFillByte);
        // read SCI MODE; if SM CANCEL is still set, repeat
        sciModeByte = sci_read(SCI_MODE);    
        if ((sciModeByte & SM_CANCEL) == 0x0000)
        {
            break;
        }
    }
    
    if ((sciModeByte & SM_CANCEL) == 0x0000)
    {    
        if(DEBUG) PRINTF("VS1053b: Song sucessfully sent. Terminating OK\r\n");
        if(DEBUG) PRINTF("VS1053b: SCI MODE = %#x, SM_CANCEL = %#x\r\n", sciModeByte, sciModeByte & SM_CANCEL);
        sci_write(SCI_DECODE_TIME, 0x0000);
    }        
    else
    {
        if(DEBUG) PRINTF("VS1053b: SM CANCEL hasn't cleared after sending 2048 bytes, do software reset\r\n");
        if(DEBUG) PRINTF("VS1053b: SCI MODE = %#x, SM_CANCEL = %#x\r\n", sciModeByte, sciModeByte & SM_CANCEL);                
        VS1053_initialize();
    }    
}

void write_plugin(const unsigned short *plugin, unsigned int len) {
    unsigned int i;
    unsigned short addr, n, val;

    for (i=0; i<len;) {
        addr = plugin[i++];
        n    = plugin[i++];
        if (n & 0x8000U) { //RLE run, replicate n samples
            n  &= 0x7FFF;
            val = plugin[i++];
            while (n--) {
                sci_write(addr,val);
            }
        } else { //copy run, copy n sample
            while (n--) {
                val = plugin[i++];
                sci_write(addr,val);
            }
        }
    }

    return;
}


bool VS1053_initialize(void) {
    config_pin();
    delayms(1);
    _reset(1);
    cs_high();                           //chip disabled
    spi_initialise();                    //initialise MBED
        
    sci_write(SCI_MODE, (SM_SDINEW+SM_RESET)); //  set mode reg.    
    delayms(10);
    
#ifdef DEBUG    
    unsigned int info = wram_read(para_chipID_0);
    if(DEBUG) PRINTF("VS1053b: ChipID_0:%04X\r\n", info);
    info = wram_read(para_chipID_1);
    if(DEBUG) PRINTF("VS1053b: ChipID_1:%04X\r\n", info);
    info = wram_read(para_version);
    if(DEBUG) PRINTF("VS1053b: Structure version:%04X\r\n", info);
#endif

    //get chip version, set clock multiplier and load patch
    int i = (sci_read(SCI_STATUS) & 0xF0) >> 4;
    if (i == 4) {
    
        if(DEBUG) PRINTF("VS1053b: Installed Chip is: VS1053\r\n");
  
        sci_write(SCI_CLOCKF, (SC_MULT_XTALIx50));
        delayms(10);
#ifdef VS_PATCH
        // loading patch
        write_plugin(vs1053b_patch, sizeof(vs1053b_patch)/2);        
    
        if(DEBUG) PRINTF("VS1053b: Patch is loaded.\r\n");
        if(DEBUG) PRINTF("VS1053b: Patch size:%d bytes\r\n",sizeof(vs1053b_patch));
        
#endif // VS_PATCH
    } 
    else 
    {
        if(DEBUG) PRINTF("VS1053b: Not Supported Chip\r\n");
        return false;
    }
    
    // change spi to higher speed 
    sdi_initialise();                
    changeVolume();
    changeBass();    
    _isIdle = true;
    return true;
}

void setVolume(float vol) 
{    
    if (vol > -0.5)
        _volume = -0.5;
    else
        _volume = vol;

    changeVolume();
}

float getVolume(void) 
{
    return _volume;
}

void setBalance(float balance) 
{    
    _balance = balance;
            
    changeVolume();
}

float getBalance(void)
{
    return _balance;    
}

void changeVolume(void) 
{
    // volume calculation        
    unsigned short volCalced = (((char)(_volume / -0.5f)) << 8) + (char)((_volume - _balance) / -0.5f);
   
    sci_write(SCI_VOL, volCalced);
    
    if(DEBUG) PRINTF("VS1053b: Change volume to %#x (%f, Balance = %f)\r\n", volCalced, _volume, _balance);        
}

int getTrebleFrequency(void)
{
    return _st_freqlimit * 1000;
}


void setTrebleFrequency(int frequency)
{
    frequency /= 1000;
    
    if(frequency < 1)
    {
        frequency = 1;
    }
    else if(frequency > 15)
    {
        frequency = 15;
    }
    _st_freqlimit = frequency;
    changeBass();
}
    
int getTrebleAmplitude(void)
{
    return _st_amplitude;
}

void setTrebleAmplitude(int amplitude)
{
    if(amplitude < -8)
    {
        amplitude = -8;
    }
    else if(amplitude > 7)
    {
        amplitude = 7;
    }
    _st_amplitude = amplitude;
    changeBass();
}   
    
int getBassFrequency(void)
{
    return _sb_freqlimit * 10;
}

void setBassFrequency(int frequency)
{
    frequency /= 10;
    
    if(frequency < 2)
    {
        frequency = 2;
    }
    else if(frequency > 15)
    {
        frequency = 15;
    }
    _sb_freqlimit = frequency;
    changeBass();
}  
    
int getBassAmplitude(void)
{
    return _sb_amplitude;
}

void setBassAmplitude(int amplitude)
{
    if(amplitude < -15)
    {
        amplitude = -15;
    }
    else if(amplitude > 0)
    {
        amplitude = 0;
    }
    _sb_amplitude = amplitude;
    changeBass();
}

void changeBass(void)
{
    unsigned short bassCalced = ((_st_amplitude  & 0x0f) << 12) 
                              | ((_st_freqlimit  & 0x0f) <<  8) 
                              | ((_sb_amplitude  & 0x0f) <<  4) 
                              | ((_sb_freqlimit  & 0x0f) <<  0);
                            
    sci_write(SCI_BASS, bassCalced);    
    
    if(DEBUG) PRINTF("VS1053b: Change bass settings to:\r\n");
    if(DEBUG) PRINTF("VS1053b: --Treble: Amplitude=%i, Frequency=%i\r\n", getTrebleAmplitude(), getTrebleFrequency());
    if(DEBUG) PRINTF("VS1053b: --Bass:   Amplitude=%i, Frequency=%i\r\n", getBassAmplitude(), getBassFrequency());
}

/*===================================================================
 * Buffer handling
 *==================================================================*/
 
unsigned int bufferLength(void)
{
    return BUFFER_SIZE; 
} 

unsigned char bufferGetByte(void)
{    
    unsigned char retVal = 0x00;
    if (bufferCount() > 0x00)
    {        
        retVal = *_bufferReadPointer++;         
        if (_bufferReadPointer >= _buffer + BUFFER_SIZE)
        {
            _bufferReadPointer = _buffer;
        }     
    }
    return retVal;
}

bool bufferSetByte(char c)
{
    if (bufferFree() > 0x00)
    {        
        *_bufferWritePointer++ = c;        
        if (_bufferWritePointer >= _buffer + BUFFER_SIZE)
        {
            _bufferWritePointer = _buffer;
        }        
        return true;
    }
    return false;
}

bool bufferPutStream(const char *s, unsigned int length)
{
    if (bufferFree() >= length)
    {
        while (length--)
        {
            *_bufferWritePointer++ = *s++;                        
            if (_bufferWritePointer >= _buffer + BUFFER_SIZE)
            {
                _bufferWritePointer = _buffer;
            }
        }
        return true;
    }
    return false;
}
    
unsigned int bufferFree(void)
{
    if(_bufferReadPointer > _bufferWritePointer)
    {
        return _bufferReadPointer - _bufferWritePointer - 1;
    }
    else if(_bufferReadPointer < _bufferWritePointer)
    {
        return BUFFER_SIZE - (_bufferWritePointer - _bufferReadPointer) - 1;
    }        
    return BUFFER_SIZE - 1;
}

unsigned int bufferCount(void)
{    
    return BUFFER_SIZE - bufferFree() - 1;
}

void bufferReset(void)
{
    _bufferReadPointer = _buffer;
    _bufferWritePointer = _buffer;            
}


void dataRequestHandler(void)
{    
    if (_isIdle && _DREQ) 
    {
        _isIdle = false;
        // write buffer to vs1053b
        unsigned length = bufferCount();        
        int i = 0;   
        sdi_en();
          
        while (length > 0)
        {
            int l2 = (length > 32) ? 32 : length;        
            //if(DEBUG) PRINTF("L2: %i\r\n", l2);    
            for( ; l2 != 0; l2--)
            {
                SPI_tbyte(bufferGetByte());            
            }
            
            length -= l2;
    
            if (!_DREQ || i > 4)
                break;    
            i++;
        }
        
        sdi_dis();   
        
        _isIdle = true;
    }               
}

void play(void)
{
   // INTERRUPT_HANDLER_ENABLE;

  
    if (_isIdle && _DREQ) 
    {
        _isIdle = false;
        // write buffer to vs1053b
        unsigned int length = bufferCount();        
        int i = 0;   
        sdi_en();
          
        while (i<4 &&length > 0)
        {
            int l2 = (length > 32) ? 32 : length;        
            //if(DEBUG) PRINTF("L2: %i\r\n", l2);    
            for( ; l2 != 0; l2--)
            {
                SPI_tbyte(bufferGetByte());            
            }
            
            length -= l2;
    
            if (!_DREQ)
                break;    
            i++;
        }
        
        sdi_dis();   
        
        _isIdle = true;
  


  }




    
}

void pause(void)
{
  //  INTERRUPT_HANDLER_DISABLE;
    if(DEBUG) PRINTF("VS1053b: Pause.\r\n");
}

void stop(void)
{
 //  INTERRUPT_HANDLER_DISABLE;
  //  __disable_irq();
    if(DEBUG) PRINTF("VS1053b: Song stoping..\r\n");
    while(!_isIdle) 
        ;
        
    // set SCI MODE bit SM CANCEL    
    unsigned short sciModeByte = sci_read(SCI_MODE);        
    sciModeByte |= SM_CANCEL;    
    sci_write(SCI_MODE, sciModeByte);
    
    // send up 2048 bytes of audio data. 
    for (int i = 0; i < 64; i++) 
    { 
        // send at least 32 bytes of audio data
        int z = bufferCount();
        if (z > 32)
            z = 32;
        for (int n = 0; n < z; n++) 
        {            
            SPI_tbyte(bufferGetByte()); 
        }
        // read SCI MODE; if SM CANCEL is still set, repeat
        sciModeByte = sci_read(SCI_MODE);    
        if ((sciModeByte & SM_CANCEL) == 0x0000)
        {
            break;
        }
    }
    
    if ((sciModeByte & SM_CANCEL) == 0x0000)
    {    
        // send at least 2052 bytes of endFillByte[7:0].
        // read endFillByte  (0 .. 15) from wram 
        unsigned short endFillByte=wram_read(para_endFillByte);
        // clear endFillByte (8 .. 15)
        endFillByte = endFillByte ^0x00FF;                          
        for (int n = 0; n < 2052; n++) 
            sdi_write(endFillByte); 
        if(DEBUG) PRINTF("VS1053b: Song sucessfully stopped.\r\n");
        if(DEBUG) PRINTF("VS1053b: SCI MODE = %#x, SM_CANCEL = %#x\r\n", sciModeByte, sciModeByte & SM_CANCEL);
        sci_write(SCI_DECODE_TIME, 0x0000);
    }
    else
    {
        if(DEBUG) PRINTF("VS1053b: SM CANCEL hasn't cleared after sending 2048 bytes, do software reset\r\n");
        if(DEBUG) PRINTF("VS1053b: SCI MODE = %#x, SM_CANCEL = %#x\r\n", sciModeByte, sciModeByte & SM_CANCEL);                
        VS1053_initialize();
    }
            
    bufferReset();  
 //   __enable_irq();  
}

void getAudioInfo(AudioInfo* aInfo)
{
    // volume calculation        
    uint16_t hdat0 = sci_read(SCI_HDAT0);
    uint16_t hdat1 = sci_read(SCI_HDAT1);
    
    if(DEBUG) PRINTF("VS1053b: Audio info\r\n");        
    
    AudioInfo* retVal = aInfo;
    retVal->type = UNKNOWN;        
    
    if (hdat1 == 0x7665)
    {
        // audio is WAV
        retVal->type = WAV;
    }  
    else if (hdat1 == 0x4154 || hdat1 == 0x4144 || hdat1 == 0x4D34 )
    {
        // audio  is AAC
        retVal->type = AAC;
    }
    else if (hdat1 == 0x574D )
    {
        // audio  is WMA
        retVal->type = WMA;
    }
    else if (hdat1 == 0x4D54 )
    {
        // audio  is MIDI
        retVal->type = MIDI;
    }
    else if (hdat1 == 0x4F76 )
    {
        // audio  is OGG VORBIS
        retVal->type = OGG_VORBIS;
    }
    else if (hdat1 >= 0xFFE0 )  //  (hdat1 >= 0xFFE0 ) &&  &&  hdat1 <= 0xFFFF
    {
        // audio  is mp3
        retVal->type = MP3_;
        
        if(DEBUG) PRINTF("VS1053b:   Audio is mp3\r\n");        
        retVal->ext.mp3.id =      (MP3_ID)((hdat1 >>  3) & 0x0003);
        switch((hdat1 >>  1) & 0x0003)
        {
        case 3:
            retVal->ext.mp3.layer = 1;    
            break;
        case 2:
            retVal->ext.mp3.layer = 2;    
            break;
        case 1:
            retVal->ext.mp3.layer = 3;    
            break;            
        default:
            retVal->ext.mp3.layer = 0;
            break;            
        }        
        retVal->ext.mp3.protrectBit =    (hdat1 >>  0) & 0x0001;                
        
        char srate =    (hdat0 >> 10) & 0x0003;       
        retVal->ext.mp3.kSampleRate = _sampleRateTable[retVal->ext.mp3.id][srate];
        
        retVal->ext.mp3.padBit =         (hdat0 >>  9) & 0x0001;
        retVal->ext.mp3.mode =(MP3_MODE)((hdat0 >>  6) & 0x0003);
        retVal->ext.mp3.extension =      (hdat0 >>  4) & 0x0003;
        retVal->ext.mp3.copyright =      (hdat0 >>  3) & 0x0001;
        retVal->ext.mp3.original =       (hdat0 >>  2) & 0x0001;
        retVal->ext.mp3.emphasis =       (hdat0 >>  0) & 0x0003;
        
        if(DEBUG) PRINTF("VS1053b:  ID: %i, Layer: %i, Samplerate: %i, Mode: %i\r\n", retVal->ext.mp3.id, retVal->ext.mp3.layer, retVal->ext.mp3.kSampleRate, retVal->ext.mp3.mode);        
    }
    
    // read byteRate
    unsigned short byteRate = wram_read(para_byteRate);
    retVal->kBitRate = (byteRate * 8) / 1000;
    if(DEBUG) PRINTF("VS1053b:  BitRate: %i kBit/s\r\n", retVal->kBitRate);
    
    // decode time
    retVal->decodeTime = sci_read(SCI_DECODE_TIME);    
    if(DEBUG) PRINTF("VS1053b:  Decodetime: %i s\r\n", retVal->decodeTime);
                  
}
