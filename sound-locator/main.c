/*
 * sound-locator
 *
 * Created: 6/4/2019 08:30:39
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "common.h"
#include "scheduler.h"

#include <avr/sleep.h>


// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }


int main(void) {
    
}
