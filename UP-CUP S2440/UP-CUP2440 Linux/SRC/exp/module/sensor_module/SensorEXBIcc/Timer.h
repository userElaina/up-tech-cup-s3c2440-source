#ifndef  _TIMER_H_
#define _TIMER_H_


extern unsigned int HoleCountH;

void timer0_init(void);
void StartCountHole(void);
void FinishCountHole(void);
unsigned int EndCountHole(void);
void timer1_init(void);
unsigned int GetHS1101(void);



#endif 

