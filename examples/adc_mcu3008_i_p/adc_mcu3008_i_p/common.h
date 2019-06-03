/*
 * common.h
 *
 * Created: 5/6/2019 22:28:35
 *  Author: danie
 */ 


#ifndef COMMON_H_
#define COMMON_H_



// Timer
void timer_init();
void timer_off();
void timer_ISR();
void timer_set(long unsigned int M);

void set_PWM(unsigned char level);
void set_PWM2(unsigned char level);
void PWM_on();
void PWM_off();


#endif /* COMMON_H_ */