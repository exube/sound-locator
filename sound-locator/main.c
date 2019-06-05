/*
 * sound-locator
 *
 * Created: 6/4/2019 08:30:39
 *
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include "common.h"
#include "scheduler.h"
#include "spi.h"

#include <avr/sleep.h>

#define BUF_SZ 32

// 0 for input, 1 for output
#define INITIALISE_PORT(port, mode) { \
    DDR##port = (uint8_t)mode; PORT##port = (uint8_t)(~mode); \
}

// bit 0 for the lsb, bit 7 for the msb
#define GET_BIT(val, bit) ((val >> bit) & 0x01)
#define SET_BIT(var, bit, val) { if (val) var |= 0x01 << bit; else var &= ~(0x01 << bit); }



// Retrieve sample from ADC and write to location dictated by current state.
// Stats for 8MHz CPU clock:
// At 5kHz sampling and mic distance 50cm, the max sliding delay between 2 mics is 1.458E-3 sec.
//  At 5kHz sampling, each sample takes 2.0E-4 secs. So, max sliding delay is 8 samples.
//  Each sample takes about 60 cycles. So, max delay error is 7.5E-6 sec.
//

// At 5ksps and mic distance 30cm, the max sliding delay between 2 mics is 8.746E-4 sec.
//  At 5ksps, each sample takes .0002 secs. So, max sliding delay is 5 samples. (not very much!)
//  Each sample takes about 60 cycles. So, max delay error is 7.5E-6 sec.
//  

// Stats for 20MHz CPU clock:
// ...

// 3 ADC inputs, 2 buffers per input.
// 16-bit AVR does not use a CPU cache, or even a pipelined CPU, so no worries about ijk accesses.
// Each pair of buffers is treated as a single 64-byte buffer by the state variable.  
// When state>buf_sz, the task sets the flag buf1_ready, signalling the calculation task that 
// it can start computing delays.
// When state>2*buf_sz, the task sets the flag buf2_ready, signalling that buffer 2 is full.
// It then resets state to 0 for the iteration.
// These flags are both cleared by the calculation task.  If the flag is still set by the time the sampler
// gets there, the sampler skips the entire buffer for 32 ticks.

#define SAMPLE_buf1_ready 0
#define SAMPLE_buf2_ready 1
#define SAMPLE_buf1_skip  2
#define SAMPLE_buf2_skip  3

#define CALC_angle_ready  4

// Our op-amp output uses a virtual ground of 0.5*V_ref
#define SAMPLE_BIAS       512
// This should be signed.
int16_t SAMPLE_mic_buf[3*2*BUF_SZ];

uint8_t sl_flags = 0;
uint16_t tick_SAMPLE(uint16_t state) {
    // Minimise delay between actual samples
    uint16_t a, b, c;
    
    a = read_sample1(0);
    b = read_sample1(1);
    c = read_sample1(2);
    
    // State transitions (simplified)
    if (state >= 2*BUF_SZ) state = 0;
    else state++;
    
    
    // State actions (simplified)
    if (state == BUF_SZ) SET_BIT(sl_flags, SAMPLE_buf1_ready, 1);
    if (state == 0) SET_BIT(sl_flags, SAMPLE_buf1_ready, 1);
    
    SAMPLE_mic_buf[0*2*BUF_SZ + state] = a - SAMPLE_BIAS;
    SAMPLE_mic_buf[1*2*BUF_SZ + state] = b - SAMPLE_BIAS;
    SAMPLE_mic_buf[2*2*BUF_SZ + state] = c - SAMPLE_BIAS;
    
    return state;
}

// Since max sliding delay is 5 samples, we will only compute xcorr for 6 samples in either direction.
// This means 13 "xcorr" array entries for each side of the triangle.
// Since a single "xcorr" performs too many operations, this machine will calculate
// 2 elements per tick.  There are 3 xcorr arrays at 13 elements each, so this machine can have an angle
// ready after 20 ticks.  (The angle calculation a simple integer divide and a trig table lookup)

#define MAX_DELAY_SAMPLES 6;
uint16_t xcorr[3*(2 * MAX_DELAY_SAMPLES + 1)];
uint16_t xcorr_AB = xcorr;
uint16_t xcorr_BC = &(xcorr[2*MAX_DELAY_SAMPLES+1]);
uint16_t xcorr_CA = &(xcorr[2*(2*MAX_DELAY_SAMPLES+1]);
uint8_t angle;

uint16_t tick_CALCULATE(uint16_t state) {
    int16_t *a_sample, *b_sample, *c_sample;
    
    // Pick a good data.
    if (GET_BIT(sl_flags, SAMPLE_buf1_ready)) {
        a_sample = &(SAMPLE_mic_buf[0*BUF_SZ]);
        b_sample = &(SAMPLE_mic_buf[2*BUF_SZ]);
        c_sample = &(SAMPLE_mic_buf[4*BUF_SZ]);
    } else if (GET_BIT(sl_flags, SAMPLE_buf2_ready)) {
        a_sample = &(SAMPLE_mic_buf[1*BUF_SZ]);
        b_sample = &(SAMPLE_mic_buf[3*BUF_SZ]);
        c_sample = &(SAMPLE_mic_buf[5*BUF_SZ]);
    } else return 0;
    
    // Take sliding dot-products of A/B
    for (int8_t offset = -MAX_DELAY_SAMPLES; offset <= MAX_DELAY_SAMPLES; offset++) {
        for (uint8_t )
    }
    
    
    
    
    return state;
}

int main(void) {
    add_task((uint16_t)-1, 1, &tick_SAMPLE);
    
    
}
