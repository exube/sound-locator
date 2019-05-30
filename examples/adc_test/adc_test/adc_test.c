/*
 * Daniel Ma <dma012@ucr.edu>
 *
 * Lab section: 023
 * Assignment: Lab  Exercise 
 * Exercise description: 
 *
 * I acknowledge all content created herein, excluding template or example code, 
 *  is my own original work.
 * Created: 5/21/2019 08:55:18
 *
 * adc_test
 * adc_test
 * Author : Daniel Ma <dma012@ucr.edu>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }
    
volatile uint8_t timer_flag = 0;
void ADC_init();
ISR(ADC_vect) {
    timer_flag = 1;
    ADC_init();
}

void ADC_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADIE);
    // Enable ADC, Start Conversion, and enable Interrupts
    
}

int main(void) {
    INITIALISE_PORT(B, 0xFF);
    INITIALISE_PORT(D, 0xFF);
    DDRA = 0x00;
    
    uint16_t sig_hi = 0;
    uint16_t cnt = 0;
    sei();
    ADC_init();
    loop:
    if (cnt >= 512) {
        uint8_t tmpb = 0;
        for (uint8_t i = 0; i < 8; i++) {
            
            if (sig_hi - 512 >= (2 << i)) {
                SET_BIT(tmpb, i, 1);
            }
        }
        /*PORTB = tmpb;
        PORTD = tmpb >> 8;*/
        PORTB = tmpb;
        cnt = 0;
        sig_hi = 0;
    } 
    while (!timer_flag);
    timer_flag = 0;
    cnt++;
    if (sig_hi < ADC) sig_hi = ADC;
    
    goto loop;
}

