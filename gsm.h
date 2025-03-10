
#ifndef GSM_H
#define GSM_H

#include "platform.h"

typedef enum
{
    GSM_MAKE_CALL,
    GSM_SEND_SMS,
    GSM_SEND_DATA
} gsm_function_t;

typedef enum
{
    GPRS_MAIN_SERVER,
    GPRS_DEBUG_SERVER,
} gprs_server_type_t;

void Gsm_Init();
void Gsm_TurnOn();
void Gsm_TurnOff();
void Gsm_SetAudioLevel(uint8_t value);
void Gsm_SetMicrophoneLevel(uint8_t value);
void Gsm_SetServerInfo(uint8_t ip1, uint8_t ip2, uint8_t ip3, uint8_t ip4, uint32_t port);

void Gsm_SendData(gprs_server_type_t serverType, uint8_t* data, uint32_t length);
void Gsm_SendOpenDoorFrame();
void Gsm_SendCloseDoorFrame();
void SendReleasedItemFrame(uint8_t receivedNote, uint8_t selectedSlot);
void SendPayoutChangeFrame(uint8_t payoutNote);
void SendSellingErrorFrame(uint8_t receivedNote, uint8_t errorSlot);
void SendMotorErrorFrame(uint8_t motorId);
void SendMotorResumeFrame();
void CreatTimeRequestFrame (void);
void RestoreEepBufferPointer();

void Gsm_MakeCall();
void Gsm_HangCall();
bool Gsm_IsCalling();

void Gsm_Process();

#endif  /* GSM_H */
