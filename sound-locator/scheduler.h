/*
 * scheduler.h
 *
 * Created: 5/23/2019 10:18:19
 *  Author: danie
 */ 


#ifndef SCHEDULER_H_
#define SCHEDULER_H_

typedef struct _task task;
typedef enum _tflag tflag;
void add_task(uint16_t state, uint32_t period, uint16_t (*tick)(uint16_t));


#endif /* SCHEDULER_H_ */