/** @file    form_enter_password.c
  * @author  
  * @version 
  * @brief   Bao gom cac chuong trinh con nhap password (keypad & lcd)
  */
#include "platform.h"
#include "glcd.h"
#include "setting_id.h"
#include "language.h"

/* from setting main */
extern uint8_t g_passwordForm;

extern void Setting_Show();


static bool g_visible = false;
static uint8_t g_userPassword[6] = {'*'};
static uint8_t g_userPasswordPointer = 0;
/**
 * @brief  Ham cap nhat hien thi Password
 * 
 * @param  NONE 
 * @retval NONE      
 */
void UpdatePassword()
{
    char tmp[7];
    /* Ve mot hinh chu nhat tai hang 35, chieu dai ca man hinh, cao 10 hang, mau den */
    GLcd_FillRect(0, 35, GLCD_WIDTH, 10, BLACK);        /* Ham nay co y nghia nhu xoa 1 vung hien thi*/
    
    for (uint8_t i = 0; i < 6; i++)
    {
        /* Neu ki tu password nhap vao khac '*' = Enter */
        if (g_userPassword[i] != '*')
        {
            /* Hien thi len man hinh LCD '*' */
            tmp[i] = '*';
        }
        /* Neu khong nhap hoac nhap ki tu '*' */
        else
        {
            /* Hien thi len man hinh LCD '-' */
            tmp[i] = '-';
        }
    }
    tmp[6] = 0;
    /* Cai dat font hien thi password la font 6x8 */
    GLcd_SetFont(&font6x8);
    /* Dua chuoi ki tu tmp vao buffer gui du lieu toi LCD, vi tri hang 35 cot 40 */
    GLcd_DrawString(tmp, 40, 35, WHITE);
    /* Gui du lieu toi man LCD */
    GLcd_FlushRegion(35, 10);
}
/**
 * @brief  Ham hien thi chuoi ki tu 'ENTER PASSWORD' va password len man hinh LCD
 * 
 * @param  language_id_t languageId -> ID chuoi ki tu muon hien thi
 * @retval NONE      
 */
void EnterPassword_Show(language_id_t languageId)
{
    char tmp[7];
    /* Set co bao goi ham hien thi man hinh nhap password */
    g_visible = true;
    /* Dua con tro ve 0 */
    g_userPasswordPointer = 0;
    /* Nhap toan bo 6 ki tu password thanh ki tu '*' */
    for (uint8_t i = 0; i < 6; i++)
    {
        g_userPassword[i] = '*';
    }
    /* Xoa man hinh LCD */
    GLcd_ClearScreen(BLACK);
    /* Tinh do dai chuoi ki tu can hien thi */
    int sizeInPixel = GLcd_MeasureStringUni(Lang_GetText(languageId));
    /* Dua chuoi ki tu vao Buffer gui du lieu toi LCD, can chinh chu giua man hinh, hang 10 */
    GLcd_DrawStringUni(Lang_GetText(languageId), (GLCD_WIDTH - sizeInPixel) / 2, 10, WHITE);
    /* Nhan password nhap tu ban phim vao g_userPassword */
    for (uint8_t i = 0; i < 6; i++)
    {
        /* Neu ki tu password nhap vao khac '*' = Enter */
        if (g_userPassword[i] != '*')
        {
            /* Hien thi len man hinh LCD '*' */
            tmp[i] = '*';
        }
        /* Neu khong nhap hoac nhap ki tu '*' */
        else
        {
            /* Hien thi len man hinh LCD '-' */
            tmp[i] = '-';
        }
    }
    tmp[6] = 0;
    /* Cai dat font hien thi password la font 6x8 */
    GLcd_SetFont(&font6x8);
    /* Dua chuoi ki tu tmp vao buffer gui du lieu toi LCD, vi tri hang 35 cot 40 */
    GLcd_DrawString(tmp, 40, 35, WHITE);
    /* Gui du lieu toi man LCD */
    GLcd_Flush();
}
/**
 * @brief  Ham xu ly su kien nhan phim nhap password
 * 
 * @param  luint8_t keyCode -> Ki tu ban phim doc duoc
 * @retval bool EnterPassword_KeyPress -> Trang thai xu ly     
 */
bool EnterPassword_KeyPress(uint8_t keyCode)
{
    /* Kiem tra co bao ham hien thi Password chua duoc set -> ham chua duoc goi */
    if (g_visible == false)
    {
        return false;   
    }
    /* Neu phim '#' = Cancel duoc an, nhap toan bo ki tu '*' vao g_userPassword */
    if (keyCode == '#')    
    {
        if(g_passwordForm == 4 || g_passwordForm == 1)
        {
          for (uint8_t i = 0; i < 6; i++)
          {
              g_userPassword[i] = '*';
          }
          /* Xoa con tro password ve 0*/
          g_userPasswordPointer = 0;
          /* hien thi menu ngoai */
          g_visible = false;
          Setting_Show();
        }
        else
        {
          for (uint8_t i = 0; i < 6; i++)
          {
              g_userPassword[i] = '*';
          }
          /* Xoa con tro password ve 0*/
          g_userPasswordPointer = 0;
          /* Thuc hien ham cap nhat hien thi password */
          UpdatePassword();
        }
    }
    /* Neu an '*' = Enter -> Khong lam gi ca */
    else if (keyCode == '*')    /* ENTER KEY */
    {
      if(g_passwordForm == 4)
      {
        g_passwordForm = 5;
        g_visible = false;
        Setting_UpdatePassword(g_userPassword);
      }
    }
    /* Neu nhan cac phim khac */
    else    
    {
      /* bo qua thai cho khi hien thi canh bao factory reset */
      if(g_passwordForm != 4)
      {
        /* Nhan tung ki tu nhap vao g_userPassword */
        g_userPassword[g_userPasswordPointer++] = keyCode;
        /* Thuc hien ham cap nhat hien thi password */
        UpdatePassword();
        /* Neu nhap du so ki tu password (6 ki tu) */
        if (g_userPasswordPointer == 6)
        {
          /* Xoa co hien thi man hinh nhap password */ /* ngoai tru luc hien thi factory reset */
          if(g_passwordForm != 3) g_visible = false;
          
          /* Chuyen sang ham so sanh password va vao giao dien cai dat */
          Setting_UpdatePassword(g_userPassword);
        }
      }
    }
    return true;
}

/**
 * @brief    Thoat menu cai dat password
 * 
 * @param    NONE
 * @retval   bool  : trang thi hien thi
 */
bool OutSettingPassWord()
{
     g_visible = false;
     return g_visible;
}