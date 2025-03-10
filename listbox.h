
#ifndef LISTBOX_H
#define LISTBOX_H

#include "platform.h"

#define LISTBOX_ITEM_MAX        32

typedef struct listbox_st listbox_t;

typedef void (*listbox_handler_t)(listbox_t* obj, uint32_t selectedIndex);

struct listbox_st
{
    int selectedIndex;
    const uint16_t* items[LISTBOX_ITEM_MAX];
    uint32_t itemNumber;
    listbox_handler_t handler;
    uint32_t drawIndex;     /* The variable is to store the index of the item that begin drawing on the screen */
};


/* Add an item into the listbox */
void Lb_AddItem(listbox_t* obj, const uint16_t* text);

/* Remove all items in the lisbox */
void Lb_Remove(listbox_t* obj);

/* Register a handler function, this function will be invoked automatically once user select an item */
void Lb_ListenEvents(listbox_t* obj, listbox_handler_t handler);

/* Draw the listbox on the screen */
void Lb_Show(listbox_t* obj);

/* Re-draw the listbox on the screen */
void Lb_Invalidate(listbox_t* obj);

/* The function used to handle the keypad events. This function must be called once a key is pressed */
void Lb_KeyPress(listbox_t* obj, uint8_t keyCode);


#endif  /* LISTBOX_H */
