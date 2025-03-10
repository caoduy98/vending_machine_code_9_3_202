
#ifndef RESOURCES_H
#define RESOURCES_H

#include "eeprom_memory_map.h"

#define RESOURCE_ITEM_MAX           100

/* page 1 sale date */
#define EEPROM_SALE_POINTER         ADDR_EEPROM_PAGE_1
/* page 2 sale year */
#define EEPROM_SALE_YEAR_POINTER    ADDR_EEPROM_PAGE_2
/* page 4 - page 7 sale year information */
#define EEPROM_SALE_START_YEAR      ADDR_EEPROM_PAGE_4
#define EEPROM_SALE_STOP_YEAR       ADDR_EEPROM_PAGE_8
/* page 10 - page 509 sale year information */
#define EEPROM_SALE_START_DAY       ADDR_EEPROM_PAGE_10
#define EEPROM_SALE_STOP_DAY        ADDR_EEPROM_PAGE_509



#define JAN_MONTH       31
#define FEB_MONTH       29  
#define MAR_MONTH       31       
#define APR_MONTH       30        
#define MAY_MONTH       31
#define JUN_MONTH       30
#define JUL_MONTH       31
#define AUG_MONTH       31
#define SEP_MONTH       30
#define OCT_MONTH       31
#define NOV_MONTH       30
#define DEC_MONTH       31

#define DAYSALE_POINTER         0
#define YEARSALE_POINTER        0

void Resource_RestoreFromMemory();
void Resource_SaveToMemory();

int Resource_GetDaylySalesfromEerom (uint8_t date,uint8_t month,uint8_t year,int* numbersale);
int Resource_GetMonthlySalesfromEerom (uint8_t month,uint8_t year,int* numbersale);
int Resource_GetYearlySalesfromEerom (uint8_t year,int* numbersale);
int Resource_GetTotalSalesfromEerom (int* numbersale);
bool Resource_SaveSalestoEeprom (int sales);
bool Resource_SaveSalesYeartoEeprom (int sales);
bool SaveTotalDaylySaletoEerom (uint16_t sale, uint16_t number);
bool SaveTotalYearlySaletoEerom (uint32_t sale, uint32_t number);
//bool SaveTotalSaletoEeprom (uint32_t sale, uint32_t number);
void SaveEepromsalepointer (void);
void ReadEepromsalepointer (void);
void SaveEepromYearsalepointer (void);
void ReadEepromYearsalepointer (void);
uint32_t calulator_poiterforgetsale (uint8_t checkdate,uint8_t checkmonth,uint8_t checkyear);

void Resource_ClearSaleDay(void);
void Resource_ClearSaleYear(void);
#endif  /* RESOURCES_H */
