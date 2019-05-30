/*
 * spi.h
 *
 * Created: 5/30/2019 09:09:17
 *  Author: danie
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void init_SPI();
void init_TFT();
void write_cmd(uint8_t data);
void write_data(uint8_t data);
void test_char();



#endif /* SPI_H_ */