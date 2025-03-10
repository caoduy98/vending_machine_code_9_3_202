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

#ifndef _VS1053_H
#define _VS1053_H

// ----------------------------------------------------------------------------
// Extended settings
// ----------------------------------------------------------------------------
//   Enable debug output (Output -> printf ...)
//   --------------------------------------------------------------------------
//   #define DEBUG
//   #define DEBUGOUT (x,y...)                printf(x, ##y);
//   Patches, Addons
//   --------------------------------------------------------------------------
//   #define VS1053_PATCH_1_4_FLAC
//   #define VS1053_PATCH_1_5
   #define VS1053_PATCH_1_5_FLAC
//   #define VS1053_SPECANA
//   #define VS1053B_PCM_RECORDER_0_9
// ----------------------------------------------------------------------------

//#include "defines.h"
#include "SPI_NXP.h"

#if defined(VS1053_PATCH_1_4_FLAC) && defined(VS1053_PATCH_1_5) && defined(VS1053_PATCH_1_5_FLAC) && defined(VS1053_SPECANA) && defined(VS1053B_PCM_RECORDER_0_9)
#error "VS1053: Exclusive use of patch and app versions."
#endif
#if defined(VS1053_PATCH_1_4_FLAC) || defined(VS1053_PATCH_1_5) || defined(VS1053_PATCH_1_5_FLAC) || defined(VS1053_SPECANA) || defined(VS1053B_PCM_RECORDER_0_9)
#define VS_PATCH
#endif

#define DEBUNG 1

#define DEFAULT_BALANCE_DIFERENCE_LEFT_RIGHT          0.0f
#define DEFAULT_VOLUME                              -40.0f
#define DEFAULT_BASS_AMPLITUDE                        5        //   0 -    15 dB
#define DEFAULT_BASS_FREQUENCY                      100        //  20 -   150 Hz
#define DEFAULT_TREBLE_AMPLITUDE                      0        //  -8 -     7 dB
#define DEFAULT_TREBLE_FREQUENCY                  15000        //1000 - 15000 Hz



// SCI register address assignment
#define SCI_MODE                                    0x00
#define SCI_STATUS                                  0x01
#define SCI_BASS                                    0x02
#define SCI_CLOCKF                                  0x03
#define SCI_DECODE_TIME                             0x04
#define SCI_AUDATA                                  0x05
#define SCI_WRAM                                    0x06
#define SCI_WRAMADDR                                0x07
#define SCI_HDAT0                                   0x08
#define SCI_HDAT1                                   0x09
#define SCI_AIADDR                                  0x0A
#define SCI_VOL                                     0x0B
#define SCI_AICTRL0                                 0x0C
#define SCI_AICTRL1                                 0x0D
#define SCI_AICTRL2                                 0x0E
#define SCI_AICTRL3                                 0x0F


//SCI_MODE register bits as of p.38 of the datasheet
#define SM_DIFF                                     0x0001
#define SM_LAYER12                                  0x0002
#define SM_RESET                                    0x0004
#define SM_CANCEL                                   0x0008
#define SM_EARSPEAKER_LO                            0x0010
#define SM_TESTS                                    0x0020
#define SM_STREAM                                   0x0040
#define SM_EARSPEAKER_HI                            0x0080
#define SM_DACT                                     0x0100
#define SM_SDIORD                                   0x0200
#define SM_SDISHARE                                 0x0400
#define SM_SDINEW                                   0x0800
#define SM_ADPCM                                    0x1000
#define SM_B13                                      0x2000
#define SM_LINE1                                    0x4000
#define SM_CLK_RANGE                                0x8000

//SCI_CLOCKF register bits as of p.42 of the datasheet
#define SC_ADD_NOMOD                                0x0000
#define SC_ADD_10x                                  0x0800
#define SC_ADD_15x                                  0x1000
#define SC_ADD_20x                                  0x1800
#define SC_MULT_XTALI                               0x0000
#define SC_MULT_XTALIx20                            0x2000
#define SC_MULT_XTALIx25                            0x4000
#define SC_MULT_XTALIx30                            0x6000
#define SC_MULT_XTALIx35                            0x8000
#define SC_MULT_XTALIx40                            0xA000
#define SC_MULT_XTALIx45                            0xC000
#define SC_MULT_XTALIx50                            0xE000

// Extra Parameter in X memory (refer to p.58 of the datasheet)
#define para_chipID_0                               0x1E00
#define para_chipID_1                               0x1E01
#define para_version                                0x1E02
#define para_config1                                0x1E03
#define para_playSpeed                              0x1E04
#define para_byteRate                               0x1E05
#define para_endFillByte                            0x1E06
//
#define para_positionMsec_0                         0x1E27
#define para_positionMsec_1                         0x1E28
#define para_resync                                 0x1E29
   
//#define INTERRUPT_HANDLER_ENABLE                    _DREQ_INTERUPT_IN.rise(this, &VS1053::dataRequestHandler); timer.attach_us(this, &VS1053::dataRequestHandler, 1000)
//#define INTERRUPT_HANDLER_DISABLE                   _DREQ_INTERUPT_IN.rise(NULL); timer.detach()  


/** Types of audio streams
 *
 */
enum AudioType 
{
    WAV,                        /*!< WAVE audio stream */
    AAC,                        /*!< AAC audio stream (ADTS (.aac), MPEG2 ADIF (.aac) and MPEG4 AUDIO (.mp4 / .m4a / .3gp / .3g2)) */
    WMA,                        /*!< Windows Media Audio (WMA) stream */
    MIDI,                       /*!< Midi audio stream */    
    OGG_VORBIS,                 /*!< Ogg Vorbis audio stream */
    MP3_,                        /*!< MPEG Audio Layer */
    UNKNOWN                     /*!< Unknown */
};

typedef enum AudioType AudioType;

/** Types of MPEG Audio Layer stream IDs.
 *
 */
enum MP3_ID
{
    MPG2_5a  = 0,               /*!< MPG 2.5, nonstandard, proprietary */
    MPG2_5b  = 1,               /*!< MPG 2.5, nonstandard, proprietary */
    MPG2_0   = 2,               /*!< ISO 13818-3 MPG 2.0 */
    MPG1_0   = 3                /*!< ISO 11172-3 MPG 1.0 */
};

typedef enum MP3_ID MP3_ID;

/** Types of MPEG Audio Layer channel modes.
 *
 */
enum MP3_MODE
{
    STEREO       = 0,           /*!< Stereo */
    JOINT_STEREO = 1,           /*!< Joint Stereo */
    DUAL_CHANNEL = 2,           /*!< Dual Channel */
    MONO         = 3            /*!< Mono */
};

typedef enum MP3_MODE MP3_MODE;
/** Struct for informations about audio streams.
 *
 */
typedef struct AudioInfo
{
   AudioType            type            : 4;        /*!< Type of the audio stream - important for the interpretation of the lower union */
   unsigned short       kBitRate;                   /*!< Average bitrate of the audio stream - in kBit/s */
   unsigned short       decodeTime;                 /*!< Decode time */
   union {
        struct {
            MP3_ID      id              : 2;        /*!< ID */
            char        layer           : 2;        /*!< Layer */
            char        protrectBit     : 1;        /*!< Protect bit, see p.44 of the datasheet */
            char        padBit          : 1;        /*!< Pad bit, see p.44 of the datasheet */
            MP3_MODE    mode            : 2;        /*!< Channel mode */
            char        extension       : 2;        /*!< Extension, see p.44 of the datasheet */
            char        copyright       : 1;        /*!< Copyright, see p.44 of the datasheet */
            char        original        : 1;        /*!< Original, see p.44 of the datasheet */
            char        emphasis        : 2;        /*!< Emphasis, see p.44 of the datasheet */
            char        kSampleRate     : 6;        /*!< Samplerate - in kHz (rounded) */
        } mp3;                                      /*!< MPEG Audio Layer */
        struct {
         //   WMA_ID      id              : 2;        /*!< ID */
            char        layer           : 2;        /*!< Layer */
            char        protrectBit     : 1;        /*!< Protect bit, see p.44 of the datasheet */
            char        padBit          : 1;        /*!< Pad bit, see p.44 of the datasheet */
         //   WMA_MODE    mode            : 2;        /*!< Channel mode */
            char        extension       : 2;        /*!< Extension, see p.44 of the datasheet */
            char        copyright       : 1;        /*!< Copyright, see p.44 of the datasheet */
            char        original        : 1;        /*!< Original, see p.44 of the datasheet */
            char        emphasis        : 2;        /*!< Emphasis, see p.44 of the datasheet */
            char        kSampleRate     : 6;        /*!< Samplerate - in kHz (rounded) */
        } wma;                                      /*!< Windows Media Audio (WMA) stream */
        struct {
        //   ACC_ID      id              : 2;        /*!< ID */
            char        layer           : 2;        /*!< Layer */
            char        protrectBit     : 1;        /*!< Protect bit, see p.44 of the datasheet */
            char        padBit          : 1;        /*!< Pad bit, see p.44 of the datasheet */
         //  ACC_MODE    mode            : 2;        /*!< Channel mode */
            char        extension       : 2;        /*!< Extension, see p.44 of the datasheet */
            char        copyright       : 1;        /*!< Copyright, see p.44 of the datasheet */
            char        original        : 1;        /*!< Original, see p.44 of the datasheet */
            char        emphasis        : 2;        /*!< Emphasis, see p.44 of the datasheet */
            char        kSampleRate     : 6;        /*!< Samplerate - in kHz (rounded) */
        } aac;                                      /*!< AAC audio stream */
        struct {
       //   OTHER_ID      id              : 2;        /*!< ID */
            char        layer           : 2;        /*!< Layer */
            char        protrectBit     : 1;        /*!< Protect bit, see p.44 of the datasheet */
            char        padBit          : 1;        /*!< Pad bit, see p.44 of the datasheet */
        //  OTHER_MODE    mode            : 2;        /*!< Channel mode */
            char        extension       : 2;        /*!< Extension, see p.44 of the datasheet */
            char        copyright       : 1;        /*!< Copyright, see p.44 of the datasheet */
            char        original        : 1;        /*!< Original, see p.44 of the datasheet */
            char        emphasis        : 2;        /*!< Emphasis, see p.44 of the datasheet */
            char        kSampleRate     : 6;        /*!< Samplerate - in kHz (rounded) */
        } other;                                    /*!< Other */
   } ext;
   
} AudioInfo;

/** Class for VS1053 - Ogg Vorbis / MP3 / AAC / WMA / FLAC / MIDI Audio Codec Chip.
 *  Datasheet, see http://www.vlsi.fi/fileadmin/datasheets/vlsi/vs1053.pdf
 *
 * This code based on:
 *  mbeduino_MP3_Shield_MP3Player
 *  http://mbed.org/users/xshige/programs/mbeduino_MP3_Shield_MP3Player/lgcx63
 *  2010-10-16
 *
 * For the use of this class, a file "defines.h" must be created. 
 * It controls debug output and vs1053 patches.
 *
 * defines.h:
 *@code
 * #ifndef _DEFINES_H
 * #define _DEFINES_H
 * // ----------------------------------------------------------------------------
 * //   debug output
 * // ----------------------------------------------------------------------------
 * // optional
 * #define DEBUG
 * #ifdef DEBUG
 * #  define DEBUGOUT(x,y...)                printf(x, ##y);
 * #else
 * #  define DEBUGOUT(x,y...)
 * #endif
 * // ----------------------------------------------------------------------------
 * //   VLSI VS1053b library, patch, apps
 * // ----------------------------------------------------------------------------
 * // optional, ONLY ONE
 * //#define VS1053_PATCH_1_4_FLAC
 * //#define VS1053_PATCH_1_5
 * #define VS1053_PATCH_1_5_FLAC
 * //#define VS1053_SPECANA
 * //#define VS1053B_PCM_RECORDER_0_9
 *
 * #endif //_DEFINES_H 
 *@endcode
 *
 * For a complete sample, see http://mbed.org/users/christi_s/programs/Lib_VS1053b
 *
 */

    /** Create a vs1053b object.
     *
     * @param mosi 
     *   SPI Master Out, Slave In pin to vs1053b.
     * @param miso 
     *   SPI Master In, Slave Out pin to vs1053b.
     * @param sck  
     *   SPI Clock pin to vs1053b.
     * @param cs   
     *   Pin to vs1053b control chip select.
     * @param rst  
     *   Pin to vs1053b reset.
     * @param dreq 
     *   Pin to vs1053b data request.
     * @param dcs  
     *   Pin to vs1053b data chip select.
     * @param buffer  
     *   Array to cache audio data.
     * @param buffer_size  
     *   Length of the array.
     */
   

   void VS1053( uint8_t *buffer,uint16_t buffer_size );

              

    /** Reset the vs1053b. (hardware reset)
     *
     */
    void reset(void);    
        
    /** Stop the playback if the song is completed. 
     *  You must call this function for default playback.
     *      
     */            
    void terminateStream(void);

    /** Initialize the vs1053b device.
     *
     * @return 
     *    TRUE on success, FALSE on failure.
     */
    bool  VS1053_initialize(void);
    
    /** Set the volume.
     * 
     * @param volume 
     *   Volume -0.5dB, -1.0dB, .. -64.0dB. 
     */    
    void  setVolume(float volume );    //  = DEFAULT_VOLUME
    
    /** Get the volume.
     *
     * @return 
     *   Return the volume in dB.
     */
    float getVolume();
    
    /** Set the balance - volume difference between left-right.
     * 
     * @param balance 
     *   Difference in dB.
     */    
    void  setBalance(float balance );         //= DEFAULT_BALANCE_DIFERENCE_LEFT_RIGHT
    
    /** Get the balance - volume difference between left-right.
     * 
     * @return
     *   Difference in dB.
     */
    float getBalance();
        
    /** Get the treble frequency limit.
     * 
     * @return
     *   Frequenzy 1000, 2000 .. 15000Hz.
     */
    int   getTrebleFrequency(void);
    /** Set the treble frequency limit.
     * 
     * @param frequency
     *   Frequenzy 1000, 2000, .. 15000Hz.
     */
    void  setTrebleFrequency(int frequency);  //  = DEFAULT_TREBLE_FREQUENCY
    
    /** Get the treble amplitude.
     * 
     * @return
     *   Amplitude -8 .. 7dB (in 1.5dB steps); 0 = off.
     */
    int   getTrebleAmplitude(void);
    /** Set the treble amplitude.
     * 
     * @param amplitude
     *   Amplitude -8 .. 7dB (in 1.5dB steps); 0 = off.
     */    
    void  setTrebleAmplitude(int amplitude );   //= DEFAULT_TREBLE_AMPLITUDE
    
     /** Get the bass frequency limit.
     * 
     * @return
     *   Frequenzy 20, 30, .. 150Hz.
     */
    int   getBassFrequency(void);
    /** Set the bass frequency limit.
     * 
     * @param frequency
     *   Frequenzy 20, 30, .. 150Hz.
     */
    void  setBassFrequency(int frequency);  //= DEFAULT_BASS_FREQUENCY
    
    /** Get the bass amplitude.
     * 
     * @return
     *   Amplitude 0 .. 15dB (in 1dB steps); 0 = off.
     */
    int   getBassAmplitude(void);
    /** Set the bass amplitude.
     * 
     * @param amplitude
     *   Amplitude 0 .. 15dB (in 1dB steps); 0 = off.
     */    
    void  setBassAmplitude(int amplitude );  //= DEFAULT_BASS_AMPLITUDE
    
    /** Set the speed of a playback.
     * 
     * @param speed
     *   Speed 0, 1, .. (0, 1 normal speed). 
     *   Speeds greater 2 are not recommended, buffer must be filled quickly enough.
     */    
    void setPlaySpeed(unsigned short speed);       
        
    /** Copy a byte into the audio data buffer.
     * 
     * @param c
     *   Data for the buffer.
     *
     * @return
     *    TRUE on success; FALSE on failure, c isn't copied in the buffer. 
     */    
    bool bufferSetByte(char c);
    
    /** Copy a array of bytes into the audio data buffer.
     * 
     * @param s
     *   Data for the buffer.
     *
     * @param length
     *  Size of data array.
     *
     * @return
     *    TRUE on success; FALSE on failure, s isn't copied in the buffer. 
     */    
    bool bufferPutStream(const char *s, unsigned int length);
    
    /** Get the free space of the audio data buffer.
     * 
     * @return
     *   Space 0 .. BUFFER_SIZE - 1.
     */    
    unsigned int bufferFree(void);
    
    /** Get the busy space of the audio data buffer.
     * 
     * @return
     *   Space 0 .. BUFFER_SIZE - 1.
     */
    unsigned int bufferCount(void); 
    
    /** Complete length of the audio buffer.
     * 
     * @return
     *   Buffer length.
     */
    unsigned int bufferLength(void);        
    
    /** Start playing audio from buffer.
     *      
     */    
    void play(void);
    
    /** Interrupt the playback.
     *      
     */    
    void pause(void);
    
    /** Stop the playback in the middle of a song. 
     *  After this call, you can now send the next audio file to buffer.
     *      
     */        
    void stop(void);
    
     /** Get information about played audio stream.
     * 
     * @param aInfo
     *   Return value for the informations.
     *     
     */    
    void getAudioInfo(AudioInfo* aInfo);


    unsigned short int wram_read(unsigned short int);
    void wram_write(unsigned short int, unsigned short int);
    void write_plugin(const unsigned short* , unsigned int);
    void cs_low(void);
    void cs_high(void);
    void dcs_low(void);
    void dcs_high(void);
    void sci_en(void);
    void sci_dis(void);
    void sdi_en(void);
    void sdi_dis(void);

    void spi_initialise(void);

    void sdi_initialise(void);

    void sci_write(unsigned char, unsigned short int);
    void sdi_write(unsigned char);
    unsigned short int sci_read(unsigned short int);
    void sine_test_activate(unsigned char);

    void sine_test_deactivate(void);

    void changeVolume(void);
        
    
    // TODO
    void power_down(void);

    void changeBass(void);
    
    unsigned char bufferGetByte(void);    
    void bufferReset(void);         
    void dataRequestHandler(void);
    
    


#endif
