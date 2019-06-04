/*
 * adc_mcu3008_i_p
 *
 * Created: 6/2/2019 11:20:59
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "common.h"
#include "scheduler.h"

#include <avr/sleep.h>
#include "spi.h"

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }

void itostr(uint16_t val, char* out, char radix) {
    const uint16_t maxsz = 7;
    uint16_t sz = 0;
    if (val == 0) {out[0] = '0'; out[1] = 0; return;}

    while (val != 0 && sz < maxsz) {
        uint8_t ones = val % 10;
        out[sz] = ones + '0';
        val /= 10;
        sz++;
    }
    for (uint16_t i = 0; i < sz / 2; i++) {
        char tmp = out[i];
        out[i] = out[(sz - i) - 1];
        out[sz - i - 1] = tmp;
    }
    
    for (uint16_t i = sz; i < maxsz; i++) {
        out[i] = ' ';
    }
    out[maxsz] = 0;
}

uint16_t greatest = 0;
uint16_t least = 1024;

// Create 2 buffers for each sample.
uint16_t bufA_1[512];
uint16_t bufB_1[512];
uint16_t bufA_2[512];
uint16_t bufB_2[512];


uint32_t tick_SAMPLE(uint32_t unused) {
    uint16_t big;
    big = read_sample1(0);
    if (big > greatest) greatest = big;
    if (big < least) least = big;
    
    return 0;
}

char bignum[8];
char smallnum[8];
uint32_t tick_SCREEN(uint32_t unused) {
    itostr(greatest, bignum, 'd');
    itostr(least, smallnum, 'd');
    
    write_strn(0, 0, bignum, 4);
    write_strn(0, 10, smallnum, 4);
    
    greatest = 0;
    least = 1024;
    
    return 0;
}

int main(void) {
    init_TFT();
    init_ADC();
    
    add_task(0, 1, &tick_SAMPLE);
    add_task(0, 512, &tick_SCREEN);
    
    timer_set(1);
    timer_init();
    
    while(1);
    
    
}
