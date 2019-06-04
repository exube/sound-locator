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
void add_task(uint32_t state, uint32_t period, uint32_t (*tick)(uint32_t));


#endif /* SCHEDULER_H_ */