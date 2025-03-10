
#ifndef NV11_DRIVER_H
#define NV11_DRIVER_H

#include "platform.h"

#define NV_TYPE_NV9         0x00
#define NV_TYPE_NV11        0x07
#define NV_TYPE_NV200       0x06
#define NOTE_NUM_PAYOUT     25

typedef enum
{
    NV11_EVENT_NONE,
    NV11_ERROR,
    NV11_NOTE_READ,
    NV11_NOTE_STACKED,
    NV11_NOTE_STORED,
    NV11_NOTE_REJECTED,
    NV11_NOTE_DISPENSED,
    NV11_NOTE_FRAUD_ATTEMPT,
    NV200_PAYOUT_FAILED,
} nv11_event_t;


int NV11_Init(void);
int NV11_GetUnitType();
int NV11_Enable(void);
int NV11_Disable(void);
bool NV11_SetAcceptNote(uint16_t channel);
int NV11_EnablePayout(void);
int NV11_DisablePayout(void);
int NV11_SetDenominationForChange(uint32_t denomination);
int NV11_RoutingPayoutForNV200(void);
int NV11_RoutingAllToCashbox(void);
int NV11_Hold(void);
int NV11_UnHold(void);
int NV11_Reject(void);
int NV11_EmptyNotes(void);
int NV11_DisplayOn(void);
int NV11_DisplayOff(void);

int NV11_GetChannelNumber();
int NV11_GetChannelValue(int channel);
int NV11_GetDenominationForChange(void);
int NV11_GetAvailableChange(void);
int NV11_GetStoredNoteByChannel(int channel);
int NV11_IsHolding(void);

nv11_event_t NV11_GetLatestEvent(void);
void NV11_ClearLatestEvent(void);
int NV11_GetLatestNote(void);
void NV11_Process(void);

/* This is the blocking function */
status_t NV11_PayoutNote(void);
int NV11_PayoutValue(int value);

bool NV200_ValueIsOnForPayout(int value);
bool NV11_GetStatusPayoutNote(void);
#endif /* NV11_DRIVER_H */
