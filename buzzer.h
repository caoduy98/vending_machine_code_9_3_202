
#ifndef BUZZER_H
#define BUZZER_H


void Buzz_Init(int pin);
void Buzz_On(void);
void Buzz_Off(void);
void Buzz_Flashing(int timeOn, int timeOff, int during);

#endif  /* BUZZER_H */
