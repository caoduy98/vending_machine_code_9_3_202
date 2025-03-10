/** @file    main.c
  * @author
  * @version
  * @brief   Chuong trinh chinh chua main while loop
  */
#include "platform.h"
#include "gsm.h"
#include "watchdog.h"

extern void MainSetup(void);
extern void MainResume();
extern void MainProcess(void);
extern void SettingSetup();
extern void SettingProcess();
extern void Door_Init();
extern bool Door_IsOpen();
extern void Door_Process();
extern void Setting_Exit();

static bool g_doorIsOpen;
extern const uint8_t* g_interface;
extern const uint8_t g_img_main_interface[];
extern const uint8_t g_img_main_interface_en[];
extern bool gPasswordInTrue;
extern bool isbanloi;

extern void CtrlMotorProviderProcess(void);
extern bool SensorOurStock_IsEmpty(void);
extern void ClearSellAndCheckEmpty(void);
extern void MotorProvideRunSwap(void);


/**
 * @brief   Chuong trinh chinh
 *
 * @param   NONE
 * @retval  NONE
 */
void main(void)
{
    /* Chay cac chuong trinh khoi tao he thong */
    PlatformInit();
    /* Kiem tra trang thai mo cua (cam bien cua) */
    g_doorIsOpen = Door_IsOpen();
    /* Neu cua dang mo */
    if (g_doorIsOpen)
    {
        /* Chay cac chuong trinh cai dat he thong */
        SettingSetup();
    }
    else
    {
        /* Neu cua dong, hien thi man hinh va goi ham xu ly su kien ban phim */
        MainSetup();
        Gsm_SendCloseDoorFrame();
    }
    while (1)
    {
        /* Goi cac chuong trinh xu ly softTimer, xu ly ban phim, xu ly GSM */
        PlatformProcess();
        /* Goi chuong trinh xu ly trang thai dong, mo cua */
        Door_Process();

        /* Check tick all time */
        Perh_ProcessAllWhile();

        /* Kiem tra neu cua da mo va dang thuc hien dong, an man hinh hien thi mat khau, chuyen sang man hinh mua hang */
        if (g_doorIsOpen)
        {
            /* Neu cua dang dong */
            if (!Door_IsOpen())
            {
                /* Gui frame dong cua toi server */
                Gsm_SendCloseDoorFrame();
                /* Cap nhat trang thai cua moi */
                g_doorIsOpen = Door_IsOpen();
                /* Thoat trinh cai dat */
                Setting_Exit();
                /* Goi ham xu ly su kien ban phim va hien thi man hinh mua hang */
                MainResume();
            }
        }
        /* Kiem tra neu cua da dong va dang mo, chuyen sang man hinh cai cai */
        else
        {
            /* Neu cua dang mo */
            if (Door_IsOpen())
            {
                /* Gui frame mo cua toi Server */
                Gsm_SendOpenDoorFrame();
                /* Cap nhat trang thai cua moi */
                g_doorIsOpen = Door_IsOpen();
                /* Goi ham cai dat va hien thi man hinh nhap password */
                SettingSetup();
               // if(!SensorOurStock_IsEmpty()){                
                isbanloi = 0; // Them 24/2/2025
                ClearSellAndCheckEmpty(); // Them 24_2_2025
                //}
            }
        }
        if (!g_doorIsOpen)
        {
            /* Neu cua dong, chay chuong trinh xu ly mua hang */
            gPasswordInTrue = false;
            MainProcess();
        }
        else
        {
            /* Neu cua mo, chay chuong trinh xu ly cai dat he thong */
            SettingProcess();
        }
        CtrlMotorProviderProcess();
        
    }
      //MotorProvideRunSwap(); // Them 4/3/2025
}
