//#include "sensors.h"
//
//
//static bool gSockSensorIsOn = false;
//static bool gAudioSensorIsOn = false;
//static bool gPirSensorIsOn = false;
//static uint8_t TimeSetSockSS = 0;
//static uint8_t TimeSetAudioSS = 0;
//static uint8_t TimeSetPirSS = 0;
//static uint8_t  gCountSetSockSS = 0;
//static uint8_t  gCountSetAudioSS = 0;
//static uint8_t  gCountSetPirSS = 0;
//static uint32_t SticktimeSockss = 0;
//static uint32_t SticktimeAudioss = 0;
//static uint32_t SticktimePirss = 0;
//static uint32_t StickBuzz = 0;
//static uint32_t StickOldTimeAlarm = 0;
//static uint32_t StickTimeState = 0;
//bool gAlarmSendFrame = false;
//bool gAlarmBuzz = false;
//bool StartBuzz = false;
//static uint8_t StateAlarm = 0;
//
//
///******************************************************************************/
//void Init_SensorIO (void)
//{
//    Pin_Init(AUDIO_SS, PIN_INPUT_PULLUP);
//    Pin_Init(SOCK_SS, PIN_INPUT_PULLDOWN);
//    Pin_Init(PIR_SS, PIN_INPUT_PULLDOWN);
//}
///******************************************************************************/
//static void Filter_noise_sensors (void)
//{
//    if(Pin_Read(SOCK_SS) == 1)TimeSetSockSS++;
//    else TimeSetSockSS = 0;
//    if((TimeSetSockSS > TimeSockFilter)&&(Pin_Read(SOCK_SS) == 1))
//    {
//        gCountSetSockSS ++;
//        TimeSetSockSS = 0;
//        if(gCountSetSockSS >= CountSensorIsOn)
//        {
//            gSockSensorIsOn = true;
//            SticktimeSockss = GetTickCount();
//            gCountSetSockSS = 0;
//        }
//    }
//    if(Pin_Read(AUDIO_SS) == 0)TimeSetAudioSS++;
//    else TimeSetAudioSS = 0;
//    if((TimeSetAudioSS > TimeAudioFilter)&&(Pin_Read(AUDIO_SS) == 0))
//    {
//        gCountSetAudioSS++;
//        TimeSetAudioSS = 0;
//        if(gCountSetAudioSS >= CountSensorIsOn)
//        {
//            gAudioSensorIsOn = true;
//            SticktimeAudioss = GetTickCount();
//            gCountSetAudioSS = 0;
//        }
//    }
//    if(Pin_Read(PIR_SS) == 1)TimeSetPirSS++;
//    else TimeSetPirSS = 0;
//    if((TimeSetPirSS > TimePirFilter)&&(Pin_Read(PIR_SS) == 1))
//    {
//        gCountSetPirSS++;
//        TimeSetPirSS = 0;
//        if(gCountSetPirSS >= CountSensorIsOn)
//        {
//            gPirSensorIsOn = true;
//            SticktimePirss = GetTickCount();
//            gCountSetPirSS = 0;
//        }
//    }
//}
///******************************************************************************/
//static bool Sock_SensorIsOn (void)
//{
//    if(gSockSensorIsOn == false)
//    {
//        return false;
//    }
//    else
//    {
//        if(GetTickCount() > SticktimeSockss + TimeHold)
//        {
//            gSockSensorIsOn = false;
//            return gSockSensorIsOn;
//        }
//        else return gSockSensorIsOn;
//    }
//}
///******************************************************************************/
//static bool Audio_SensorIsOn (void)
//{
//    if(gAudioSensorIsOn == false)
//    {
//        return false;
//    }
//    else
//    {
//        if(GetTickCount() > SticktimeAudioss + TimeHold)
//        {
//            gAudioSensorIsOn = false;
//            return gAudioSensorIsOn;
//        }
//        else return gAudioSensorIsOn;
//    }
//}
///******************************************************************************/
//static bool Pir_SensorIsOn (void)
//{
//    if(gPirSensorIsOn == false)
//    {
//        return false;
//    }
//    else
//    {
//        if(GetTickCount() > SticktimePirss + TimeHold)
//        {
//            gPirSensorIsOn = false;
//            return gPirSensorIsOn;
//        }
//        else return gPirSensorIsOn;
//    }
//}
///******************************************************************************/
//static void MBuzz_On (uint32_t TimeAlarm)
//{
//    if(!StartBuzz)StickOldTimeAlarm = GetTickCount();
//
//    if(GetTickCount() > StickOldTimeAlarm + TimeAlarm)
//    {
//        Pin_Write(BEEP_PIN,0);
//        gAlarmBuzz = false;
//        StartBuzz = false;
//    }
//    else
//    {
//        StartBuzz = true;
//        if(GetTickCount() > StickBuzz + 100)
//        {
//            StickBuzz = GetTickCount();
//            Pin_Toggle(BEEP_PIN);
//        }
//    }
//}
///******************************************************************************/
//static void MBuzz_Pip (void)
//{
//    if(!StartBuzz)StickOldTimeAlarm = GetTickCount();
//
//    if(GetTickCount() > StickOldTimeAlarm + 200)
//    {
//        Pin_Write(BEEP_PIN,0);
//        StartBuzz = false;
//        StateAlarm = 1;
//    }
//    else
//    {
//        StartBuzz = true;
//        Pin_Write(BEEP_PIN,1);
//    }
//}
///******************************************************************************/
//static void MBuzz_Off (void)
//{
//    Pin_Write(BEEP_PIN,0);
//    StartBuzz = false;
//}
///******************************************************************************/
//void Alarm_Process (void)
//{
//    Filter_noise_sensors();
//}
///******************************************************************************/
//void AlarmWithDoorIsClose (void)
//{
//   StateAlarm = 0;
//   if(Pir_SensorIsOn())
//   {
//      gAlarmBuzz = true;
//      gAlarmSendFrame = true;
//   }
//   else
//   {
//      if(Audio_SensorIsOn()&&Sock_SensorIsOn())
//      {
//          gAlarmBuzz = true;
//      }
//   }
//   if(gAlarmBuzz)MBuzz_On(TimeAlarmByBuzz);
//   else MBuzz_Off();
//}
///******************************************************************************/
//void AlarmWithDoorIsOpen (bool PasswordIsTrue)
//{
//    if(Pir_SensorIsOn()&&(!PasswordIsTrue))
//    {
//        if(StateAlarm == 0)
//        {
//            MBuzz_Pip();
//            StickTimeState = GetTickCount();
//        }
//        if(StateAlarm == 1)
//        {
//            if(GetTickCount() > StickTimeState + TimeforInPassword)
//            {
//                gAlarmBuzz = true;
//                gAlarmSendFrame = true;
//            }
//        }
//    }
//    if(PasswordIsTrue)
//    {
//        gAlarmBuzz = false;
//        gAlarmSendFrame = false;
//        StateAlarm = 0;
//        MBuzz_Off();
//    }
//    if(gAlarmBuzz)MBuzz_On(TimeAlarmByBuzz);
//}
///******************************************************************************/
