/****************************************************************************
 * @file     display.c
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/
#include <stdint.h>
#include "display.h"
#include "tm4c.h"
#include "pet.h"

#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64
#define DISPLAY_PAGES   (DISPLAY_HEIGHT / 8) //8

#define DISPLAY_CTRL_CMD    0x00
#define DISPLAY_CTRL_DATA   0x40

/* on-board LEDs bitmasks for Port A */
#define LED_RED     (1U << 2)     //PA2

/* on-board LEDs bitmasks for Port B */
#define SCL         (1U << 2)     //PB2
#define SDA         (1U << 3)     //PB3

static const uint8_t init_cmds[] = {
    0xAE,
    0xD5, 0x80,
    0xA8, 0x3F,
    0xD3, 0x00,
    0x40,
    0x8D, 0x14,
    0x20, 0x00,
    0xA1,
    0xC8,
    0xDA, 0x12,
    0x81, 0xCF,
    0xD9, 0xF1,
    0xDB, 0x40,
    0xA4,
    0xA6,
    0xAF,
};

void display_init(){
    //PB - I2C Display
    GPIOB_AHB->AFSEL |= (SCL | SDA); //PB2, PB3
    GPIOB_AHB->PCTL  |= (3U << 8) | (3U << 12); //both 3 bits, PMC2 bit 8, PMC3 bit 12
    GPIOB_AHB->DEN   |= (SCL | SDA);
    GPIOB_AHB->ODR   |= SDA; //open-drain operation

    I2C0->MCR = 0x10;   //master mode
    I2C0->MTPR = 24;     // for ~100 kHz at 16 MHz
}


void display_init_seq(){
    uint32_t i = 0;
    volatile int d = 0;

    for(i = 0; i < sizeof(init_cmds); i++){
        I2C0->MSA = (0x3C << 1);    
        I2C0->MDR = DISPLAY_CTRL_CMD;   //control byte
        I2C0->MCS = 0x03;               //start + stop
        while(I2C0->MCS & 0x01);
        for(d = 0; d < 1000; d++);

        I2C0->MDR = init_cmds[i];   //command from init_cmds
        I2C0->MCS = 0x05;           //run + stop
        while(I2C0->MCS & 0x01);
        for(d = 0; d < 1000; d++);

        if (I2C0->MCS & 0x02) {
            GPIOA_AHB->DATA = LED_RED;
            while (1);
        }
    }
}

static void display_write(uint8_t control, const uint8_t *buf, uint32_t len){ // (pointer to first byte to send, how many bytes to send)
    uint32_t i = 0;
    volatile int d = 0;

    I2C0->MSA = (0x3C << 1);
    I2C0->MDR = control;           //control byte = data
    I2C0->MCS = 0x03;           //start + run
    while(I2C0->MCS & 0x01);
    for(d = 0; d < 1000; d++);

    for(i = 0; i < len; i++){
        I2C0->MDR = buf[i];
        I2C0->MCS = (i == len-1) ? 0x05 : 0x01; //stop (0x05) on last byte
        while(I2C0->MCS & 0x01);
    }
}


void display_clear(){
    volatile int d = 0;
    volatile int n = 0;
    int page = 0;

    //col adr and page adr
    uint8_t window[] = {0x21, 0x00, 0x7F, 0x22, 0x00, 0x07};
    display_write(DISPLAY_CTRL_CMD, window, sizeof(window));

    //data mode, set window
    uint8_t zeros[DISPLAY_WIDTH] = {0}; //all 0's (screen is 128 pixels; 128 cols, 8 high -> 64/8 = 8 pages)
    for(page = 0; page < DISPLAY_PAGES; page++){
        display_write(DISPLAY_CTRL_DATA, zeros, DISPLAY_WIDTH);
    }

}

//test func for drawing a pixel by coordinate
void display_set_pixel(uint8_t x, uint8_t y){
    //calc bit
    //set window
    //write to byte

    //check bounds
    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
        return;

    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    uint8_t addr[] = {
        0x21, x, x,
        0x22, page, page
    };
    display_write(DISPLAY_CTRL_CMD, addr, sizeof(addr));

    uint8_t pixel = (1U << bit);
    display_write(DISPLAY_CTRL_DATA, &pixel, 1);

}


void display_draw_pet(Pet_t *pet){
    //
}


