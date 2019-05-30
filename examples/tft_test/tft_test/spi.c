/*
 * spi.c
 *
 * Created: 5/30/2019 08:46:01
 *  Author: danie
 */ 

#include "spi.h"
#include <avr/io.h>

/* User Configuration */

// Data/Command pin
#define DC_PORT PORTD
#define DC_DDR  DDRD
#define DC_PIN  0


/* End User Configuration */


#define     CS_PORT     PORTB
#define     DDR_SPI     DDRB
#define     DD_CS       DDRB4
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









void init_SPI() {
    // Configure MOSI, SCK, CS as output, all others input.
    DDR_SPI = (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_CS);
    // SPI, Master, clock rate at fck/4
    // We want double speed
    SPSR = 1 << SPI2X;
    
    SPCR = (1 << SPE) | (1 << MSTR);
}

void init_TFT() {
    
    DC_DDR |= 1 << DC_PIN;
    DC_PORT &= ~(1 << DC_PIN);
    
    init_SPI();
    CS_PORT |= 1 << DD_CS;
    
    
    
}

void write_cmd(uint8_t data) {
    // Zero DC to send command, and pull CS low to enable the slave
    DC_PORT &= ~(1 << DC_PIN);
    CS_PORT &= ~(1 << DD_CS); 
    SPDR = data;
    // Wait for TX complete
    while (!(SPSR & (1 << SPIF)));
    
    // Sleep the slave
    CS_PORT |= 1 << DD_CS; 
}

void write_data(uint8_t data) {
    DC_PORT |= (1 << DC_PIN);
    CS_PORT &= ~(1 << DD_CS); 
    
    SPDR = data;
    // Wait for TX complete
    while (!(SPSR & (1 << SPIF)));
    
    CS_PORT |= 1 << DD_CS;
}


static const uint8_t initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI_PWCTR1      , 1, 0x23,             // Power control VRH[5:0]
  ILI_PWCTR2      , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI_VMCTR1      , 2, 0x3e, 0x28,       // VCM control
  ILI_VMCTR2  , 1, 0x86,             // VCM control2
  ILI_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI_VSCRSA  , 1, 0x00,             // Vertical scroll zero
  ILI_PIXSET  , 1, 0x66,
  ILI_FRMCTR1 , 2, 0x00, 0x18,
  ILI_DISCTRL , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI_GAMSET , 1, 0x01,             // Gamma curve selected
  ILI_DGMCTR1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI_DGMCTR2 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI_SLPOUT  , 1,  0x80,                // Exit Sleep
  ILI_DISPON  , 1,  0x80,                // Display on
  ILI_NOP                                   // End of list
};


 const uint8_t test_a[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0,
    0, 1, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 1, 1, 0, 0,
    0, 0, 1, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 0, 1, 0, 0,
    0, 0, 1, 1, 1, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};
    
// Write a test char to pixel column 16-24, row 16-24
void test_char() {
    /*
    write_cmd(ILI_SWRES);
    write_cmd(ILI_NOP);
    
    write_cmd(ILI_NORON);
    write_cmd(ILI_DINVON);
    write_cmd(ILI_DISPON);
    */
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
    //write_cmd(ILI_DINVON);
    
    write_cmd(ILI_RAMWR);
    for (uint8_t j = 0; j < 240; j++) {
        for (uint16_t k = 0; k < 320; k++) {
            write_data(0);
            write_data(0);
            write_data(0);
        }            
    }
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
    /*
    for (i = 0; i < 64; i++) {
        for (uint8_t j = 0; j < 64; j++) {
            if (test_a[((i/8)*8) + j/8]) {
                write_data(252); // R
                write_data(0); // G
                write_data(0); // B
            } else {
                write_data(0);
                write_data(0);
                write_data(0);
            }
        }                
    }
    */
    
    for (i = 0; i < 64; i++) {
        if (test_a[i]) {
            write_data(252);
            write_data(0);
            write_data(0);
        } else {
            write_data(252);
            write_data(252);
            write_data(252);
        }
    }
    
    write_cmd(ILI_NOP);
}


