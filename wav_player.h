
#ifndef WAV_PLAYER_H
#define WAV_PLAYER_H

#include "platform.h"


void Wav_Init();
void Wav_Play(const char* wav_file);
bool Wav_IsPlaying();
void Wav_Stop();
void Wav_SetVolume(uint8_t level);
void Wav_Process();

#endif /* WAV_PLAYER_H */
