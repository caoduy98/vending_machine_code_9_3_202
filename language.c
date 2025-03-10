/** @file    language.c
  * @author
  * @version
  * @brief   Khoi tao font space English and Vietnam (!Note 16bit), detection and select language
  */

#include "platform.h"
#include "language.h"
#include "peripheral.h"
#include "eeprom.h"
/**
 * @brief  Mang 2 chieu chua chuoi ki tu hien thi cua 2 ngon ngu Tieng anh vA Tieng Viet
 *
 * @param  LANG_ID -> Vi tri theo hang (Ki tu can hien thi)
 * @param  LANG_END -> Vi tri theo cot (Ngon ngu can hien thi)
 * @retval uint16_t* g_languageStrings -> Chuoi ki tu trich xuat tu mang
 */
const uint16_t* g_languageStrings[LANG_ID_MAX][LANG_END] =
{
    { u"NH\x1EACP M\x1EACT KH\x1EA8U", u"ENTER PASSWORD" },
    { u"NH\x1EACP M\x1EACT KH\x1EA8U M\x1EDAI", u"ENTER NEW PASSWORD" },
    { u"NH\x1EACP L\x1EA0I M\x1EACT KH\x1EA8U", u"ENTER IT AGAIN" },
    { u"M\x1EACT KH\x1EA8U KH\xD4NG KH\x1EDAP", u"PASSWORD NOT MATCH" },
    { u"XONG", u"DONE" },
    { u"T\x1ED4NG DOANH THU", u"TOTAL SALES" },
    { u"THEO NG\xC0Y", u"DAYLY SALES" },
    { u"THEO TH\xC1NG", u"MONTHLY SALES" },
    { u"THEO N\x102M", u"YEARLY SALES" },
    { u"KEY WITH SLOT", u"KEY WITH SLOT" },
    { u"C\xC0I \x110\x1EB6T S\x1ED0 L\x1AF\x1EE2NG", u"CAPACITY" },
    { u"\x110\x1A0N GI\xC1", u"PRICE" },
    { u"S\x1ED0 L\x1AF\x1EE2NG", u"CAPACITY" },
    { u"\x42\x1EACT T\x1EAET \x110K \x110\x1ED8 \x1EA8M", u"HUMIDITY ON/OFF" },
    { u"C\x1EA2M BI\x1EBEN R\x1A0I", u"SET DROP SENSOR" },
    { u"TH\xD4NG TIN L\x1ED6I", u"ERROR LOG" },
    { u"L\x1ED6I MOTOR", u"MOTOR ERROR" },
    { u"L\x1ED6I TH\x41NH TO\xC1N", u"PAYMENT SYSERR" },
    { u"\x58\xD3\x41 L\x1ED6I \x110\x1ED8NG \x43\x1A0", u"CLEAR MOTOR ERROR" },
    { u"\x58\xD3\x41 L\x1ED6I PH\x1EA6N \x43\x1EE8NG",  u"CLEAR HWARE ERROR" },
    { u"KH\xD4NG \x43\xD3 L\x1ED6I", u"NO ERROR" },
    { u"\x110\x1EA6U \x110\x1ECC\x43 \x42\x1ECA L\x1ED6I", u"PAYMENT SYS IS ERROR" },
    { u"KI\x1EC2M TRA KHAY H\xC0NG", u"TEST SLOT" },
    { u"TI\x1EC0N V\xC0O RA", u"CASH TOTAL DATA" },
    { u"T\x1ED4NG TI\x1EC0N V\xC0O", u"TOTAL CASH IN" },
    { u"T\x1ED4NG TI\x1EC0N RA", u"TOTAL CASH OUT" },
    { u"\x58\xD3\x41 S\x1ED0 \x44\x1AF", u"CLEAR BALANCE" },
    { u"C\xC0I \x110\x1EB6T \x110\x1ED8 \x1EA8M", u"SET HUMIDITY" },
    { u"C\xC0I \x110\x1EB6T TH\x1EDCI GIAN", u"SET TIME" },
    { u"TI\x1EC0N TR\x1EA2 L\x1EA0I", u"BILL FOR CHANGE" },
    { u"B\x1EACT/T\x1EAET", u"ON/OFF" },
    { u"\x110\x1AF\x41 TI\x1EC0N V\xC0O", u"BILL FILL" },
    { u"L\x1EA4Y TI\x1EC0N RA", u"EMPTY RECYCLER" },
    { u"XEM TH\xD4NG TIN", u"VIEW INFO" },
    { u"S\x1ED0 L\x1AF\x1EE2NG \x110\x1ED8NG C\x1A0", u"SET SLOT NUMBER" },
    { u"S\x1ED0 C\x1ED8T:", u"COLUMNS:" },
    { u"S\x1ED0 H\xC0NG:", u"ROWS:" },
    { u"M\x1EC6NH GI\xC1 CH\x1EA4P NH\x1EACN", u"ACCEPTABLE NOTE" },
    { u"M\x1EC6NH GI\xC1 TR\x1EA2 L\x1EA0I", u"REFUND NOTE" },
    { u"C\xC0I \x110\x1EB6T L\x1ED6I", u"SET NV11 ERROR" },
    { u"NG\xD4N NG\x1EEE", u"LANGUAGE" },
    { u"TI\x1EBENG VI\x1EC6T", u"VIETNAMESE" },
    { u"TI\x1EBENG ANH", u"ENGLISH" },
    { u"\x110\x1ED4I M\x1EACT KH\x1EA8U", u"CHANGE PASSWORD" },
    { u"XEM ID M\xC1Y", u"VIEW ID" },
    { u"XEM IP S\x45RV\x45R", u"VIEW SERVER INFOR" },
    { u"C\xC0I \x110\x1EB6T N\xC2NG CAO", u"ADVANCE SETTING" },
    { u"S\x1ED0 L\x1AF\x1EE2NG:", u"QUANTITY:" },
    { u"T\x1ED4NG TI\x1EC0N:", u"AMOUNT:" },
    { u"B\x1EACT", u"TURN ON" },
    { u"T\x1EAET", u"TURN OFF" },
    { u"RESET V\x1EC0 M\x1EB6\x43 \x110\x1ECANH", u"FACTORY RESET" },
    { u"\x110\x110ANG G\x1ECCI", u"CALLING" },
    { u"KH\xD4NG TR\x1EA2 L\x1EDCI", u"NO ANSWER" },
    { u"M\xC1Y \x110\x41NG \x42\x1EACN", u"NUMBER IS BUSY" },
    { u"M\x1EA4T T\xCDN HI\x1EC6U", u"SIGNAL IS LOST" },
    { u"\x43U\x1ED8\x43 G\x1ECCI \x42\x1ECA L\x1ED6I", u"CALLING ERROR" },
    { u"S\x1ED0 M\xC1Y T\x1ED4NG \x110\xC0I", u"OPERATOR NUMBER" },
    { u"C\xC0I \x110\x1EB6T \xC2M THANH", u"AUDIO SETTING" },
    { u"\x42\x1EACT/T\x1EAET", u"ON/OFF" },
    { u"\xC2M L\x1AF\x1EE2NG CH\xCDNH", u"MAIN VOLUME" },
    { u"\xC2M L\x1AF\x1EE2NG CU\x1ED8\x43 G\x1ECCI", u"CALL VOLUME" },
    { u"\x110\x1ED8 NH\x1EA0Y MIC", u"MIC GAIN LEVEL" },
    { u"TI\x1EC0N B\x1ECA K\x1EB8T", u"NOTE JAMMED"},
    /* */
    { u"CH\x1EA0Y NG\x102N", u"TESTING SLOT"},
    { u"NG\x102N L\x1ED6I", u"SLOT ERROR"},
    /* */
    { u"PHI\xCAN B\x1EA2N PH\x1EA6N M\x1EC0M", u"VIEW FIRMWARE"},
    { u"PHI\xCAN B\x1EA2N HI\x1EC6N T\x1EA0I", u"CURRENT FIRMWARE VERSION"},
    /* */
    { u"KH\x1EDEI T\x1EA0O THAM S\x1ED0", u"INIT PARAMETER"},
    { u"\x110\x1ED4I M\x1EACT KH\x1EA8U I", u"CHANGE PWORD I" },
    { u"\x110\x1ED4I M\x1EACT KH\x1EA8U II", u"CHANGE PWORD II" },
    /* */
    { u"L\x1ED6I C\x1EA2M BI\x1EBEN R\x1A0I", u"DROP SENSOR ERROR"},
    { u"\x58\xD3\x41 L\x1ED6I C\x1EA2M BI\x1EBEN", u"CLEAR DSENSOR ERROR"},
    /* */
    /* khong thu hien chinh sua duoi dong nay */
    { u"C\xD3         KH\xD4NG", u"YES           NO"},
    { u"B\x1EA0N MU\x1ED0N TI\x1EBEP T\x1EE4\x43 ?", u"CONTINUE RESET ?"},
    { u"TGIAN C\x1EA4P H\x1AF\x1A0NG", u"None Time"},
    { u"S\x1ED0 H\x1AF\x1A0NG B\xC1N", u"NTWO Capacity"},
    { u"S\x1ED0 H\x1AF\x1A0NG BTH\xCAM", u"NTHREE Capacity"},
    {u"\x58\xD3\x41 L\x1ED6I SELL", u"CLEAR ERROR SELL"},
};

static language_t g_currentLanguage = LANG_VNI;         /* Ngon ngu khai bao ban dau: Tieng Viet*/

/**
 * @brief  Ham doc ngon ngu cai dat trong EEPROM
 *
 * @param  LANGUAGE_EEP_ADDRESS -> Dia chi EEPROM chu du lieu cai dat ngon ngu
 * @retval uint16_t* g_languageStrings -> Chuoi ki tu trich xuat tu mang
 */
void Lang_RestoreFromMemory()
{
    uint8_t data;
    /* Lay du lieu tu EEPROM */
    bool res = EE_ReadByte(LANGUAGE_EEP_ADDRESS, &data);
    /* Neu doc thanh cong va du lieu = 0*/
    if (res == true && data == 0)
    {
        g_currentLanguage = LANG_VNI;   /* Ngon ngu Tieng Viet */
    }
    /*Neu khong doc duoc hoac du lieu khac 0 */
    else if (data == 1)
    {
        g_currentLanguage = LANG_ENG;   /* Ngon ngu Tieng Anh */
    }
}
/**
 * @brief  Ham cai dat ngon ngu vao EEPROM
 *
 * @param  LANGUAGE_EEP_ADDRESS -> Dia chi EEPROM chu du lieu cai dat ngon ngu
 * @param  language_t language -> Du lieu cua ngon ngu cai dat
 * @retval NONE
 */
void SetCurrentLanguage(language_t language)
{
    if (language < LANG_END)
    {
        g_currentLanguage = language;
        /* Ghi vao EEPROM du lieu ngon ngu tai dia chi da khai bao */
        EE_WriteByte(LANGUAGE_EEP_ADDRESS, g_currentLanguage);
    }
}
/**
 * @brief  Ham lay gia tri ngon ngu hien tai
 *
 * @param  NONE
 * @retval language_t GetCurrentLanguage -> Ngon ngu hien tai
 */
language_t GetCurrentLanguage()
{
    return g_currentLanguage;
}

/**
 * @brief  Ham lay chuoi ki tu trong mang 2 chieu theo ngon ngu hien tai
 *
 * @param  language_id_t langId -> Dia chi hang chua chuoi ki tu
 * @param  g_currentLanguage -> Vi tri cot ngon ngu hien tai
 * @retval uint16_t* Lang_GetText -> Chuoi ki tu can doc
 */
const uint16_t* Lang_GetText(language_id_t langId)
{
    return g_languageStrings[langId][g_currentLanguage];
}
