/** @file    listbox.c
  * @author  
  * @version 
  * @brief   Cung cap cac ham de quan ly cac chuc nang ban phim va
  *          hien thi LCD khi hien thi menu tren man hinh
  */

/*********************************************************************************
 * INCLUDE
 */
#include "platform.h"
#include "listbox.h"
#include "glcd.h"

/*********************************************************************************
 * DEFINE
 */
/* The high in pixel for a item */
#define LISTBOX_ITEM_HIGH       16
#define LISTBOX_ITEM_TOP        17
#define LISTBOX_ITEMS_PER_PAGE  3

/*********************************************************************************
 * EXPORTED FUNCTION
 */

/**
 * @brief    Hien thi con tro tam giac vao g_offBuffer
 * 
 * @param    int x  -  vi tri hien thi con tro theo chieu ngang
 * @param    int y  -  vi tri hien thi con tro theo chieu doc
 * 
 * @retval   NONE
 */
void DrawPointer(int x, int y)
{
    GLcd_FillRect(x, y, 1, 7, WHITE);
    GLcd_FillRect(x + 1, y + 1, 2, 5, WHITE);
    GLcd_FillRect(x + 3, y + 2, 2, 3, WHITE);
    GLcd_FillRect(x + 5, y + 3, 2, 1, WHITE);
}

/**
 * @brief    Them text vao listbox
 * 
 * @param    listbox_t* obj        -  con tro cau truc quan ly du lieu hien thi LCD
 * @param    const uint16_t* text  -  con tro den du lieu text menu hien thi tren LCD
 * 
 * @retval   NONE
 */
void Lb_AddItem(listbox_t* obj, const uint16_t* text)
{
    assert(obj != NULL);

    if (obj->itemNumber < LISTBOX_ITEM_MAX)
    {
        obj->items[obj->itemNumber] = text;
        obj->itemNumber++;
    }
}

/**
 * @brief    Xoa du lieu trong listbox
 * 
 * @param    listbox_t* obj        -  con tro cau truc quan ly du lieu hien thi LCD
 * @retval   NONE
 */
void Lb_Remove(listbox_t* obj)
{
    assert(obj != NULL);

    obj->itemNumber = 0;
    obj->selectedIndex = 0;
    obj->drawIndex = 0;
}

/**
 * @brief    Them ham callback khi nhan phim Enter
 * 
 * @param    listbox_t* obj              -  con tro cau truc quan ly du lieu hien thi LCD
 * @param    listbox_handler_t handler   -  con tro den ham xu ly du lieu
 * @retval   NONE
 */
void Lb_ListenEvents(listbox_t* obj, listbox_handler_t handler)
{
    assert(obj != NULL);

    obj->handler = handler;
}

/**
 * @brief    Ham hien thi du lieu trong listbox
 * 
 * @param    listbox_t* obj  -  con tro cau truc quan ly du lieu hien thi LCD
 * @retval   NONE
 */
void Lb_Show(listbox_t* obj)
{
    assert(obj != NULL);
    /* ve background */
    GLcd_FillRect(0, LISTBOX_ITEM_TOP, GLCD_WIDTH, GLCD_HEIGHT - LISTBOX_ITEM_TOP, BLACK);
    /* ve items trong listbox */
    uint32_t i = 0;
    for (i = obj->drawIndex; i < obj->itemNumber; i++)
    {
        GLcd_DrawStringUni(obj->items[i], 10, LISTBOX_ITEM_TOP + (i - obj->drawIndex) * LISTBOX_ITEM_HIGH, WHITE);
    }
    /* ve con tro hien thi */
    DrawPointer(0, LISTBOX_ITEM_TOP + (obj->selectedIndex - obj->drawIndex) * LISTBOX_ITEM_HIGH + 5);
    /* dua du lieu tu bo dem g_offBuffer len LCD */
    GLcd_Flush();
}

/**
 * @brief    Ham hien thi du lieu trong listbox khong bao gom dong dau tien
 * 
 * @param    listbox_t* obj  -  con tro cau truc quan ly du lieu hien thi LCD
 * @retval   NONE
 */
void Lb_Invalidate(listbox_t* obj)
{
    assert(obj != NULL);
    /* ve background */
    GLcd_FillRect(0, LISTBOX_ITEM_TOP, GLCD_WIDTH, GLCD_HEIGHT - LISTBOX_ITEM_TOP, BLACK);
    /* Draw the items */
    uint32_t i = 0;
    for (i = obj->drawIndex; i < obj->itemNumber; i++)
    {
        GLcd_DrawStringUni(obj->items[i], 10, LISTBOX_ITEM_TOP + (i - obj->drawIndex) * LISTBOX_ITEM_HIGH, WHITE);
    }
    /* Draw the pointer */
    DrawPointer(0, LISTBOX_ITEM_TOP + (obj->selectedIndex - obj->drawIndex) * LISTBOX_ITEM_HIGH + 5);
    GLcd_FlushRegion(LISTBOX_ITEM_TOP, GLCD_HEIGHT - LISTBOX_ITEM_TOP);
}

/**
 * @brief    Ham hien thi listbox theo phim nhan
 * 
 * @param    listbox_t* obj  -  con tro cau truc quan ly du lieu hien thi LCD
 * @param    uint8_t keyCode -  keycode of keypad
 * @retval   NONE
 */
void Lb_KeyPress(listbox_t* obj, uint8_t keyCode)
{
    assert(obj != NULL);

    if (keyCode == '8')     /* UP ARROW */
    {
        obj->selectedIndex--;
        if (obj->selectedIndex < 0)
        {
            obj->selectedIndex = obj->itemNumber - 1;
        }

        /* Scroll the items if needed */
        if (obj->selectedIndex < obj->drawIndex)
        {
            obj->drawIndex = obj->selectedIndex;
        }
        else if ((obj->selectedIndex == obj->itemNumber - 1) && (obj->itemNumber > 2))
        {
            obj->drawIndex = obj->selectedIndex + 1 - LISTBOX_ITEMS_PER_PAGE;
        }
        else if ((obj->selectedIndex == obj->itemNumber - 1) && (obj->itemNumber <= 2))
        {
          obj->drawIndex = 0;
        }
        /* hien thi lai menu */
        Lb_Invalidate(obj);
    }
    else if (keyCode == '0')    /* DOWN ARROW */
    {
        obj->selectedIndex++;
        if (obj->selectedIndex >= obj->itemNumber)
        {
            obj->selectedIndex = 0;
        }

        /* Scroll the items if needed */
        if (obj->selectedIndex >= obj->drawIndex + LISTBOX_ITEMS_PER_PAGE)
        {
            obj->drawIndex = obj->selectedIndex + 1 - LISTBOX_ITEMS_PER_PAGE;
        }
        else if (obj->selectedIndex == 0)
        {
            obj->drawIndex = 0;
        }
        /* hien thi lai menu */
        Lb_Invalidate(obj);
    }
    else if (keyCode == '*')    /* ENTER */
    {
        if (obj->handler)
        {
            obj->handler(obj, obj->selectedIndex);
        }
    }
}

