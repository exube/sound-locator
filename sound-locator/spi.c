/*
 * spi.c
 *
 * Created: 5/30/2019 08:46:01
 *  Author: danie
 */ 

#include "spi.h"
#include <avr/io.h>

#include "font8x8_basic.h"

/* User Configuration */

// Data/Command pin
#define DC_PORT PORTD
#define DC_DDR  DDRD
#define DC_PIN  0


/* End User Configuration */


#define     CS_PORT     PORTB
#define     DDR_SPI     DDRB

#define     DD_CS_ADC1  DDRB2
#define     DD_CS_ADC2  DDRB3   // Maybe unneeded
#define     DD_CS_TFT   DDRB4
#define     DD_MOSI     DDRB5
#define     DD_MISO     DDRB6
#define     DD_SCK      DDRB7



//// Commands ////

// Util/Setup
#define     ILI_NOP     0x00
#define     ILI_SWRES   0x01
#define     ILI_SLPIN   0x10
#define     ILI_SLPOUT  0x11
#define     ILI_NORON   0x13
#define     ILI_DINVOFF 0x20
#define     ILI_DINVON  0x21
#define     ILI_GAMSET  0x26
#define     ILI_DISPOFF 0x28
#define     ILI_DISPON  0x29

// Set column limits for MCU
#define     ILI_CASET   0x2A
// Set page (row) limits for MCU
#define     ILI_PASET   0x2B  

#define     ILI_RAMWR   0x2C

#define     ILI_PWCTR1  0xC0
#define     ILI_PWCTR2  0xC1
#define     ILI_VMCTR1  0xC5
#define     ILI_VMCTR2  0xC7

#define     ILI_MADCTL  0x36
#define     ILI_VSCRSA  0x37
#define     ILI_PIXSET  0x3A
#define     ILI_FRMCTR1 0xB1
#define     ILI_DISCTRL 0xB6
#define     ILI_DGMCTR1 0xE2    // Digital Gamma Ctrl
#define     ILI_DGMCTR2 0xE3    // Digital Gamma Ctrl


#define txwait() while (!(SPSR & (1 << SPIF)))
#define set_data() DC_PORT |= (1 << DC_PIN)
#define set_cmd() DC_PORT &= ~(1 << DC_PIN)
#define tft_desel() CS_PORT |= 1 << DD_CS_TFT
#define tft_sel() CS_PORT &= ~(1 << DD_CS_TFT)
#define adc1_sel()  CS_PORT &= ~(1 << DD_CS_ADC1)
#define adc1_desel() CS_PORT |= (1 << DD_CS_ADC1)
#define adc2_sel()  CS_PORT &= ~(1 << DD_CS_ADC2)
#define adc2_desel() CS_PORT |= (1 << DD_CS_ADC2)

static const uint8_t initcmd[] = {
    //0xEF, 3, 0x03, 0x80, 0x02,
    //0xCF, 3, 0x00, 0xC1, 0x30,
    //0xED, 4, 0x64, 0x03, 0x12, 0x81,
    //0xE8, 3, 0x85, 0x00, 0x78,
    //0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    //0xF7, 1, 0x20,
    //0xEA, 2, 0x00, 0x00,
    ILI_PWCTR1      , 1, 0x23,             // Power control VRH[5:0]
    ILI_PWCTR2      , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
    ILI_VMCTR1      , 2, 0x3e, 0x28,       // VCM control
    ILI_VMCTR2  , 1, 0x86,             // VCM control2
    ILI_MADCTL  , 1, 0x48,             // Memory Access Control
    ILI_VSCRSA  , 1, 0x00,             // Vertical scroll zero
    ILI_PIXSET  , 1, 0x55,             // 16-bit color mode (5/6/5)
    //ILI_FRMCTR1 , 2, 0x00, 0x18,       // Frame rate set
    ILI_DISCTRL , 3, 0x08, 0x82, 0x27, // Display Function Control
    ILI_GAMSET , 1, 0x01,             // Gamma curve selected
    ILI_DGMCTR1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    ILI_DGMCTR2 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    ILI_SLPOUT  , 1,  0x80,                // Exit Sleep
    ILI_DISPON  , 1,  0x80,                // Display on
    ILI_NOP                                   // End of list
};

uint8_t SPI_AVAILABLE = 0;

void init_SPI() {
    if (SPI_AVAILABLE) return;
    
    // Configure MOSI, SCK, CS as output, all others input.
    DDR_SPI = (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_CS_TFT) | (1 << DD_CS_ADC1) | (1 << DD_CS_ADC2);
    // SPI, Master, clock rate at fck/4
    // We want double speed
    SPSR = 1 << SPI2X;
    
    SPCR = (1 << SPE) | (1 << MSTR);
    SPI_AVAILABLE = 1;
}

void init_TFT() {
    
    DC_DDR |= 1 << DC_PIN;
    set_cmd();
    
    init_SPI();
    tft_desel();
    
    
    // Send initial commands
    uint8_t i = 0;
    while (i < sizeof(initcmd)) {
        write_cmd(initcmd[i]);
        if (initcmd[i] == ILI_NOP) break;
        i++;
        
        uint8_t numargs = initcmd[i];
        i++;
        for (uint8_t j = 0; j < numargs; j++) {
            write_data(initcmd[i]);
            i++;
        }
    }
    write_cmd(ILI_RAMWR);
    for (uint8_t j = 0; j < 240; j++) {
        for (uint16_t k = 0; k < 320; k++) {
            write_data(0);
            write_data(0);
                
        }
    }
}

void init_ADC() {
    adc1_desel();
    adc2_desel();
    init_SPI();
}

// Please only send 1 or 2.
uint16_t read_sample1(uint8_t channel) {
    //uint16_t result = 0;
    uint8_t result_high;
    uint8_t result_low;
    adc1_sel();
    // Data send: 0000 0001 (Start bit)
    SPDR = 0x01;
    
    // Data send: S/D D2 D1 D0 X X X X
    // where D[2:0] = channel, and S/D = 1 (single ADC)
    // Data recv: ? ? ? ? ? 0 B[9:8]
    channel |= 0x08; // Set the S/D bit
    channel = channel << 4;

    txwait();
    
    SPDR = channel; // Shift to allow the adc to send the null bit before transmission
    txwait();
    result_high = SPDR & 0x03;
    // Data send: X X X X X X X X
    // Data recv: B[9:2]
    // In our implementation, we drop B[1:0] for faster calculation.
    SPDR = 0x00;
    txwait();
    result_low = SPDR;
    
    // Throw away the rest of the transmission
    //SPDR = 0x00;
    //txwait();
    
    adc1_desel();
    return (((uint16_t)result_high) << 8) | result_low;
}



void write_cmd(uint8_t data) {
    // Zero DC to send command, and pull CS low to enable the slave
    set_cmd();
    tft_sel(); 
    SPDR = data;
    // Wait for TX complete
    txwait();
    
    // Sleep the slave
    tft_desel(); 
}

void write_data(uint8_t data) {
    set_data();
    tft_sel(); 
    
    SPDR = data;
    // Wait for TX complete
    txwait();
    
    tft_desel();
}

// Write a single char to the position on the screen.
void write_char(uint16_t col, uint16_t pg, char data) {
    // We'll use an optimized inline of write cmd/data to minimise idle time and 
    // eliminate superfluous changes to chip select.
    set_cmd();
    tft_sel();
    SPDR = ILI_CASET;
    txwait();
    
    set_data();
    SPDR = col >> 8;
    txwait();
    SPDR = col;
    txwait();
    
    col += 7;
    set_data();
    SPDR = col >> 8;
    txwait();
    SPDR = col;
    txwait();
    
    // Set page (row) boundries
    set_cmd();
    SPDR = ILI_PASET;
    txwait();
    
    set_data();
    SPDR = pg >> 8;
    txwait();
    SPDR = pg;
    txwait();
    
    pg += 7;
    SPDR = pg >> 8;
    txwait();
    SPDR = pg;
    txwait();
    
    set_cmd();
    SPDR = ILI_RAMWR;
    txwait();
    
    set_data();
    
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t row = font8x8_basic[(uint8_t)data][i];
        for (uint8_t j = 0; j < 8; j++) {
            if (row & 0x01) {
                SPDR = 0xFF;
                txwait();
                SPDR = 0xFF;
                txwait();
            } else {
                SPDR = 0x00;
                txwait();
                SPDR = 0x00;
                txwait();
            }
            row >>= 1;
        }
    }
    
    // Sleep the slave
    tft_desel();
    
}

void write_strn(uint16_t col, uint16_t pg, const char *str, uint8_t sz) {
    while (*str && sz) {
        write_char(col, pg, *str);
        col += 8;
        sz--;
        str++;
    }
}

void set_col(uint16_t col_l, uint16_t col_r) {
    write_cmd(ILI_CASET);
    write_data(col_l >> 8);
    write_data(col_l);
    write_data(col_r >> 8);
    write_data(col_r);
    
}
void set_row(uint16_t row_n, uint16_t row_s) {
    write_cmd(ILI_PASET);
    write_data(row_n >> 8);
    write_data(row_n);
    write_data(row_s >> 8);
    write_data(row_s);
}
void prep_write() {
    write_cmd(ILI_RAMWR);
}

void write_pixbyte(uint8_t data) {
    for (uint8_t j = 0; j < 8; j++) {
        if (data & 0x01) {
            write_data(0xFF);
            write_data(0xFF);
        } else {
            write_data(0x00);
            write_data(0x00);
        }
        data = data >> 1;
    }
    tft_desel();
}

// Write a test char to pixel column 16-24, row 16-24
void test_char() {
    uint8_t i = 0;
    while (i < sizeof(initcmd)) {
        write_cmd(initcmd[i]);
        if (initcmd[i] == ILI_NOP) break;
        i++;
        
        uint8_t numargs = initcmd[i];
        i++;
        for (uint8_t j = 0; j < numargs; j++) {
            write_data(initcmd[i]);
            i++;
        }
    }
    
    write_cmd(ILI_RAMWR);
    for (uint8_t j = 0; j < 240; j++) {
        for (uint16_t k = 0; k < 320; k++) {
            write_data(0);
            write_data(0);
            
        }            
    }
    return;
    write_cmd(ILI_NOP);
    write_cmd(ILI_CASET);
    write_data(0);
    write_data(0x10);
    write_data(0);
    write_data(0x17);
    
    write_cmd(ILI_PASET);
    write_data(0);
    write_data(0x10);
    write_data(0);
    write_data(0x17);
    
    write_cmd(ILI_RAMWR);
    
    
    uint8_t* test_a = font8x8_basic[0x62];
    for (i = 0; i < 8; i++) {
        uint8_t row = test_a[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (row & 0x01) {
                write_data(255);
                write_data(255);
            } else {
                write_data(0);
                write_data(0);
            }
            row >>= 1;
        }            
    }
    
    write_cmd(ILI_NOP);
}


