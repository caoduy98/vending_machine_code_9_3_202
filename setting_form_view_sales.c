/** @file    setting_form_view_slave.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu doanh so ban hang
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "language.h"

/*********************************************************************************
 * DEFINE
 */
#define TIME_SETTING_OFFSET_X     25
#define TIME_SETTING_OFFSET_Y     17

/*********************************************************************************
 * EXTERN FUNCTION
 */
extern void Setting_Show();

/*********************************************************************************
 * STATIC VARIABLE
 */

static int g_combindId;
static bool g_visible = false;
static int g_date = 0, g_month = 0, g_year = 0,g_yeardisplay;
static uint32_t g_totalSales = 0;
static uint32_t g_soldItemNumber = 0;

/*********************************************************************************
 * STATIC FUNCTION
 */

/**
 * @brief    Hien thi doanh so ban hang len tren LCD 
 * 
 * @param    NONE
 * @retval   NONE
 */
static void Paint()
{
//    char str[10];
    /* Draw background */
    GLcd_ClearScreen(BLACK);
    /* ve huong dan nhan phim tren dong tren cung */
    DrawTopGuideLine();
    GLcd_SetFont(&font6x8);
    g_yeardisplay = g_year + 2000;
    if (g_combindId == ID_SALE_IN_DATE)
    {
        /* hien thi ngay-thang-nam trong che do xem doanh so ban theo ngay */
        sprintf(strLcd, "%02d-%02d-%04d", g_date, g_month, g_yeardisplay);
        int sizeInPixel = GLcd_MeasureString(strLcd);
        GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, TIME_SETTING_OFFSET_Y, WHITE);
    }
    else if (g_combindId == ID_SALE_IN_MONTH)
    {
        /* hien thi thang-nam trong che do xem doanh so ban theo thang */
        sprintf(strLcd, "%02d-%04d", g_month, g_yeardisplay);
        int sizeInPixel = GLcd_MeasureString(strLcd);
        GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, TIME_SETTING_OFFSET_Y, WHITE);
    }
    else if (g_combindId == ID_SALE_IN_YEAR)
    {
        /* hien thi nam trong che do xem doanh so ban theo nam */
        sprintf(strLcd, "%04d", g_yeardisplay);
        int sizeInPixel = GLcd_MeasureString(strLcd);
        GLcd_DrawString(strLcd, (GLCD_WIDTH - sizeInPixel) / 2, TIME_SETTING_OFFSET_Y, WHITE);
    }
    else if (g_combindId == ID_SALE_IN_TOTAL)
    {
        /* hien thi title tong doang so ban hang */
        int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_TOTAL_SALES));
        GLcd_DrawStringUni(Lang_GetText(LANG_TOTAL_SALES), (GLCD_WIDTH - sizeInPixel) / 2, TIME_SETTING_OFFSET_Y, WHITE);
    }
    
    /* hien thi text tong so hang da ban */
    int quantitySizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_QUANTITY));
    int amountSizeInPixel = GLcd_MeasureStringUni(Lang_GetText(LANG_AMOUNT));
    int tmp = 0;
    if (quantitySizeInPixel > amountSizeInPixel)
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_QUANTITY), 0, TIME_SETTING_OFFSET_Y + 15, WHITE);
        GLcd_DrawStringUni(Lang_GetText(LANG_AMOUNT), quantitySizeInPixel - amountSizeInPixel, TIME_SETTING_OFFSET_Y + 30, WHITE);
        tmp = quantitySizeInPixel + 4;
    }
    else
    {
        GLcd_DrawStringUni(Lang_GetText(LANG_QUANTITY), amountSizeInPixel - quantitySizeInPixel, TIME_SETTING_OFFSET_Y + 15, WHITE);
        GLcd_DrawStringUni(Lang_GetText(LANG_AMOUNT), 0, TIME_SETTING_OFFSET_Y + 30, WHITE);
        tmp = amountSizeInPixel + 4;
    }
    /* hien thi tong so mat hang da ban */
    GLcd_SetFont(&font6x8);
    sprintf(strLcd, "%d", g_soldItemNumber);
    GLcd_DrawString(strLcd, tmp, TIME_SETTING_OFFSET_Y + 21, WHITE);
    
    /* hien thi tong so tien */
    sprintf(strLcd, "%d", g_totalSales);
    GLcd_DrawString(strLcd, tmp, TIME_SETTING_OFFSET_Y + 36, WHITE);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi doanh so ban hang tren LCD
 * 
 * @param    uint32_t id - lua chon hien thi doanh so ban hang theo ngay, thang, nam
 *                         hoac theo tong so hang
 *
 * @retval   NONE
 */
void SettingViewSales_Show(uint32_t id)
{
    /* Initialization variable */
    g_visible = true;
    g_combindId = id;
     /* lay thoi gian thuc */
    Rtc_GetCurrentDate(&g_date, &g_month, &g_year);
    /* lay doanh so ban hang theo che do xem */
    if (g_combindId == ID_SALE_IN_DATE)
    {
        //g_totalSales = Resource_GetDaylySales(g_date, g_month, g_year, &g_soldItemNumber);
        g_totalSales = Resource_GetDaylySalesfromEerom(g_date, g_month, g_year, &g_soldItemNumber);
    }
    else if (g_combindId == ID_SALE_IN_MONTH)
    {
        //g_totalSales = Resource_GetMonthlySales(g_month, g_year, &g_soldItemNumber);
        g_totalSales = Resource_GetMonthlySalesfromEerom(g_month, g_year, &g_soldItemNumber);
    }
    else if (g_combindId == ID_SALE_IN_YEAR)
    {
        //g_totalSales = Resource_GetYearlySales(g_year, &g_soldItemNumber);
        g_totalSales = Resource_GetYearlySalesfromEerom(g_year, &g_soldItemNumber);
    }
    else if (g_combindId == ID_SALE_IN_TOTAL)
    {
        //g_totalSales = Resource_GetTotalSales(&g_soldItemNumber);
        g_totalSales = Resource_GetTotalSalesfromEerom(&g_soldItemNumber);
    }
    /* Hien thi menu doanh so ban hang */
    Paint();
}

/**
 * @brief    Ham xu ly du lieu khi co phim nhan luc LCD dang hien thi doanh so ban hang
 * 
 * @param    uint8_t keyCode   -    Key code of keypad
 * @retval   bool    true      -    Neu thuc hien xu ly thanh cong
 *                   false     -    Neu menu khong hien thi hoac nhan phim Cancel
 */
bool SettingViewSales_KeyPress(uint8_t keyCode)
{
    /* neu khong hien thi menu thi tra ve false  */
    uint8_t date_max = 0;
    if (g_visible == false)
    {
        return false;
    }
    /* neu nhan phim cancel thi tra ve false  va hien thi menu */
    if (keyCode == '#')             /* CANCEL KEY */
    {
        g_visible = false;
        /* hien thi menu */
        Setting_Show();
        return true;
    }
    else if (keyCode == '*')       /* ENTER KEY */
    {
        
    }
    else if (keyCode == '0')      /* DOWN ARROW */
    {
        /* Trong che do xem theo ngay thi giam thoi gian hien thi theo ngay */
        if (g_combindId == ID_SALE_IN_DATE)
        {
            g_date--;
            if (g_date < 1)
            {
                g_month--;
                switch(g_month)
                {
                    case 12: {g_date = DEC_MONTH;break;}
                    case 11: {g_date = NOV_MONTH;break;} 
                    case 10: {g_date = OCT_MONTH;break;}
                    case 9 : {g_date = SEP_MONTH;break;}
                    case 8 : {g_date = AUG_MONTH;break;}
                    case 7 : {g_date = JUL_MONTH;break;}
                    case 6 : {g_date = JUN_MONTH;break;}
                    case 5 : {g_date = MAY_MONTH;break;}
                    case 4 : {g_date = APR_MONTH;break;}
                    case 3 : {g_date = MAR_MONTH;break;}
                    case 2 : {g_date = FEB_MONTH;break;}
                    case 1 : {g_date = JAN_MONTH;break;}
                }
                if (g_month < 1)
                {
                    g_month = 12;
                    if (g_year > 0)
                    {
                        g_year--;
                    }
                }
            }
            /* Lay tong doanh so ban hang trong ngay */
            //g_totalSales = Resource_GetDaylySales(g_date, g_month, g_year, &g_soldItemNumber);
            g_totalSales = Resource_GetDaylySalesfromEerom(g_date, g_month, g_year, &g_soldItemNumber);
        }
        else if (g_combindId == ID_SALE_IN_MONTH)
        {
            /* Trong che do xem theo thang thi giam thoi gian hien thi theo thang */
            g_month--;
            if (g_month < 1)
            {
                g_month = 12;
                if (g_year > 0)
                {
                    g_year--;
                }
            }
            /* Lay tong doanh so ban hang trong thang */
            //g_totalSales = Resource_GetMonthlySales(g_month, g_year, &g_soldItemNumber);
            g_totalSales = Resource_GetMonthlySalesfromEerom(g_month, g_year, &g_soldItemNumber);
        }
        else if (g_combindId == ID_SALE_IN_YEAR)
        {
            int dd, mm, yy;
            Rtc_GetCurrentDate(&dd, &mm, &yy);
            g_year--;
            if (g_year < yy - 10)
            {
                g_year = yy;
            }
            /* Lay tong doanh so ban hang trong nam */
            //g_totalSales = Resource_GetYearlySales(g_year, &g_soldItemNumber);
            g_totalSales = Resource_GetYearlySalesfromEerom(g_year, &g_soldItemNumber);
        }
    }
    else if (keyCode == '8')    /* UP ARROW */
    {
        if (g_combindId == ID_SALE_IN_DATE)
        {
            /* Trong che do xem theo ngay thi tang thoi gian hien thi theo ngay */
            int dd, mm, yy;
            Rtc_GetCurrentDate(&dd, &mm, &yy);
            if (Rtc_CompareDate(g_date, g_month, g_year, dd, mm, yy) < 0)
            {
                g_date++;
                switch(g_month)
                {
                    case 12: {date_max = DEC_MONTH;break;}
                    case 11: {date_max = NOV_MONTH;break;} 
                    case 10: {date_max = OCT_MONTH;break;}
                    case 9 : {date_max = SEP_MONTH;break;}
                    case 8 : {date_max = AUG_MONTH;break;}
                    case 7 : {date_max = JUL_MONTH;break;}
                    case 6 : {date_max = JUN_MONTH;break;}
                    case 5 : {date_max = MAY_MONTH;break;}
                    case 4 : {date_max = APR_MONTH;break;}
                    case 3 : {date_max = MAR_MONTH;break;}
                    case 2 : {date_max = FEB_MONTH;break;}
                    case 1 : {date_max = JAN_MONTH;break;}
                }
                if (g_date > date_max)
                {
                    g_date = 1;
                    g_month++;
                    if (g_month > 12)
                    {
                        g_month = 1;
                        if(g_year < yy)
                        {
                            g_year++;
                        }
                    }
                }
            }
            /* Lay tong doanh so ban hang trong ngay */
            //g_totalSales = Resource_GetDaylySales(g_date, g_month, g_year, &g_soldItemNumber);
            g_totalSales = Resource_GetDaylySalesfromEerom(g_date, g_month, g_year, &g_soldItemNumber);
        }
        else if (g_combindId == ID_SALE_IN_MONTH)
        {
            /* Trong che do xem theo ngay thi tang thoi gian hien thi theo ngay */
            int dd, mm, yy;
            Rtc_GetCurrentDate(&dd, &mm, &yy);
            if (Rtc_CompareDate(dd, g_month, g_year, dd, mm, yy) < 0)
            {
                g_month++;
                if (g_month > 12)
                {
                    g_month = 1;
                    if(g_year < yy)
                    {
                        g_year++;
                    }
                }
            }
            /* Lay tong doanh so ban hang trong thang */
            //g_totalSales = Resource_GetMonthlySales(g_month, g_year, &g_soldItemNumber);
            g_totalSales = Resource_GetMonthlySalesfromEerom(g_month, g_year, &g_soldItemNumber);
        }
        else if (g_combindId == ID_SALE_IN_YEAR)
        {
            int dd, mm, yy;
            Rtc_GetCurrentDate(&dd, &mm, &yy);
            if (g_year < yy)
            {
                g_year++;
            }
            /* Lay tong doanh so ban hang trong nam */
            //g_totalSales = Resource_GetYearlySales(g_year, &g_soldItemNumber);
            g_totalSales = Resource_GetYearlySalesfromEerom(g_year, &g_soldItemNumber);
        }
    }
    /* Hien thi menu doanh so ban hang */
    Paint();
    return true;
}

/**
 * @brief    Thoat menu hien thi doanh so ban hang
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi menu doanh so ban hang
 */
bool OutSettingViewSales()
{
     g_visible = false;
     return g_visible;
}
