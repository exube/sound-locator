/*
 * common.c
 *
 * Created: 5/23/2019 10:23:57
 *  Author: danie
 */ 


#include "common.h"

#include <avr/io.h>
#include <avr/interrupt.h>
uint32_t avr_timer_max = 1;
uint32_t avr_timer_current = 0;


// For this project, max period needs to be 1600 cycles (for 5kHz sampling at 8MHz CPU clock).  
// This also allows me to update 1 8x8 area on the display per "tick". (1 character, or 1/64 of the 64x64 "compass")
// The tasks must be rigorously ordered.  Sampling takes 60 +/- 3 (constant) cycles, and must be done
// first.  The screen update takes about 1200+/-100 cycles (constant).  This leaves about 200 cycles
// for processing (xcorr, trig).  These tasks will be fit into a <1600 cycle period.
// Other costs: subroutine call induces ~8 cycles of overhead, depending on args and return.
// Interrupt takes ~8 cycles, depending on architecture.  

// 
void timer_init() {
    // WGM1[3:0] = 4 (CTC mode)
    // CS1[2:0] = 3 (clk/64 prescaler)
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CS1[2:0] = 3

    // Interrupt every 25*64 = 1600 cycles. 
    OCR1A = 25;
    TIMSK1 = 1 << OCIE1A; // bit1: OCIE1A -- enables compare match interrupt

    //Initialize avr counter
    TCNT1=0;
    
    avr_timer_current = avr_timer_max;
    sei();
}
void timer_off() {
    TCCR1B = 0x00;
}
void timer_set(unsigned long M) {
    avr_timer_max = M;
    avr_timer_current = avr_timer_max;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
    // CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
    avr_timer_current--; // Count down to 0 rather than up to TOP
    if (avr_timer_current == 0) { // results in a more efficient compare
        timer_ISR(); // Call the ISR that the user uses
        avr_timer_current = avr_timer_max;
    }
}

#ifdef TCCR0B
// PWM
// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(uint8_t level) {
    OCR0A = level;
}
void set_PWM2(uint8_t level) {
    OCR0B = level;
}


void PWM_on() {
    // WGM0[2:0] = 3: Fast PWM Mode, overflow mode (TOP = 0xFF, BOTTOM = 0x00)
    // COM0A[1:0] = 2: Clear OC0A on Compare Match, Set OC0A at BOTTOM
    TCCR0A = (3 << WGM00) | (2 << COM0A0) | (2 << COM0B0);
    
    // CS[2:0] = 3: Clock from prescaler /64
    TCCR0B = (3 << CS00);
    TCNT0 = 0;
    OCR0A = 0x00; // 0% duty
    
    
}

void PWM_off() {
    TCCR0A = 0;
}
#endif
