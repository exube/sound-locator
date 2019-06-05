/*
 * scheduler.c
 *
 * Created: 5/23/2019 10:38:27
 *  Author: danie
 */

//#include <avr/io.h>
#include <stdint.h>
#include "common.h" 
#include "scheduler.h"

#define TASK_MAX 8
uint8_t task_i = 0;

struct _task {
    uint16_t state;
    uint32_t period;
    uint32_t elapsed;
    uint16_t (*tick)(uint16_t);
    //tflag flags;
};

enum _tflag {
    TF_ENABLE = 1 << 0, 
    TF_REPEAT = 1 << 1
};

task tasks[TASK_MAX];

void add_task(uint16_t state, uint32_t period, uint16_t (*tick)(uint16_t)) {
    tasks[task_i].state = state;
    tasks[task_i].period = period;
    tasks[task_i].elapsed = 0;
    tasks[task_i].tick = tick;
    task_i++;
}

void timer_ISR() {
    for (uint8_t i = 0; i < task_i; i++) {
        tasks[i].elapsed++;
        if (tasks[i].elapsed >= tasks[i].period) {
            tasks[i].elapsed = 0;
            tasks[i].state = (*tasks[i].tick)(tasks[i].state);
        }
    }
}
