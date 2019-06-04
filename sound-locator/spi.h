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
void init_ADC();
void write_cmd(uint8_t data);
void write_data(uint8_t data);
void test_char();

void write_char(uint16_t col, uint16_t pg, char data);
void write_strn(uint16_t col, uint16_t pg, const char *str, uint8_t sz);

uint16_t read_sample1(uint8_t channel);


#endif /* SPI_H_ */