/*
 * tft_test
 *
 * Created: 5/30/2019 08:26:00
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "common.h"
#include "scheduler.h"
#include "spi.h"
#include <avr/sleep.h>





// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }


int main(void) {
    // SPI as all output except MISO (uC configured)
    INITIALISE_PORT(B, 0xFF);
    
    // For Data/Command
    INITIALISE_PORT(D, 0xFF); 
    
    INITIALISE_PORT(A, 0x00);
    INITIALISE_PORT(C, 0xFF);
    PORTC = 0;
    init_TFT();
    PORTC = 0xFF;
    test_char();

    while(1) {
        PORTC = PINA;
    }
    
}
