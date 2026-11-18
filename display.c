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

static const uint8_t ssd1306_font5x7[6][5] = { //11 rows, 5 colums
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x04, 0x02, 0x7F, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x22, 0x41, 0x49, 0x49, 0x36}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
};

static const uint8_t ssd1306_font5x7_chars[52][5] = {
    {0x7E, 0x09, 0x09, 0x09, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x49, 0x3A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x30, 0x40, 0x40, 0x40, 0x3F}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x02, 0x04, 0x08, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x1E, 0x21, 0x21, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x09, 0x09, 0x76}, // R
    {0x26, 0x49, 0x49, 0x49, 0x32}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x10, 0x20, 0x7F}, // W
    {0x41, 0x22, 0x1C, 0x22, 0x41}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z

    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x44, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x44}, // c
    {0x38, 0x44, 0x44, 0x44, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x04, 0x04, 0x7E, 0x05, 0x05}, // f
    {0x08, 0x54, 0x54, 0x54, 0x3C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x78, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x14, 0x7C}, // q
    {0x00, 0x7C, 0x08, 0x04, 0x04}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x04, 0x3F, 0x44, 0x44}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
};

void display_init(){
    //PB - I2C Display
    GPIOB_AHB->AFSEL |= (SCL | SDA); //PB2, PB3
    GPIOB_AHB->PCTL  |= (3U << 8) | (3U << 12); //both 3 bits, PMC2 bit 8, PMC3 bit 12
    GPIOB_AHB->DEN   |= (SCL | SDA);
    
    GPIOB_AHB->ODR   &= ~SCL;
    GPIOB_AHB->ODR   |= SDA;  //open-drain operation

    I2C0->MCR = 0x10;   //master mode
    I2C0->MTPR = 24;     // for ~100 kHz at 16 MHz
}

static int I2C_wait(void){
    volatile int d;
    for(d = 0; d < 20; d++){} //let BUSY assert
    while(I2C0->MCS & 0x01){}

    if(I2C0->MCS & 0x02){ //error
        I2C0->MCS = 0x04; //stop
        while(I2C0->MCS & 0x01){}
        return -1;
    }
    return 0;
}

void display_init_seq(){
    //one transaction
    display_write(DISPLAY_CTRL_CMD, init_cmds, sizeof(init_cmds));
}

void display_write(uint8_t control, const uint8_t *buf, uint32_t len){ // (pointer to first byte to send, how many bytes to send)
    uint32_t i = 0;

    I2C0->MSA = (0x3C << 1);
    I2C0->MDR = control;           //control byte = data
    I2C0->MCS = 0x03;           //start + run
    if(I2C_wait()){
        GPIOA_AHB->DATA = LED_RED;
        while(1){}
    }

    for(i = 0; i < len; i++){
        I2C0->MDR = buf[i];
        I2C0->MCS = (i == len-1) ? 0x05 : 0x01; //run+stop (0x05) on last byte
        
        if(I2C_wait()){
            GPIOA_AHB->DATA = LED_RED;
            while(1){}
        }
    }
}

void display_fill(uint8_t value){
    uint8_t window[] = {0x21, 0x00, 0x7F, 0x22, 0x00, 0x07};
    uint8_t line[DISPLAY_WIDTH];
    int i, page;

    for(i = 0; i < DISPLAY_WIDTH; i++){
        line[i] = value;
    }

    display_write(DISPLAY_CTRL_CMD, window, sizeof(window));
    for(page = 0; page < DISPLAY_PAGES; page++){
        display_write(DISPLAY_CTRL_DATA, line, DISPLAY_WIDTH);
    }

}

void display_clear(void){

    int page;
    uint8_t zeros[DISPLAY_WIDTH]; //all 0's (screen is 128 pixels; 128 cols, 8 high -> 64/8 = 8 pages)

    //col adr and page adr
    int i = 0;

    for(i = 0; i < DISPLAY_WIDTH; i++){ //for 128 pixels, clear
        zeros[i] = 0x00;
    }

    for(page = 0; page < DISPLAY_PAGES; page++){ //for all pages

        uint8_t window[] = {
            0x21, 0x00, 0x7F, //0-127 cols
            0x22, page, page
        };
        
        display_write(DISPLAY_CTRL_CMD, window, sizeof(window));

        display_write(DISPLAY_CTRL_DATA, zeros, DISPLAY_WIDTH);
    }

    //data mode, set window
}

void display_draw_digit(uint8_t x, uint8_t page, uint8_t digit){
    
    if(digit > 5) return; // out of bound

    uint8_t addr[] = {
        0x21, x, (uint8_t)(x+4), 
        0x22, page, page
    };

    display_write(DISPLAY_CTRL_CMD, addr, sizeof(addr));
    display_write(DISPLAY_CTRL_DATA, ssd1306_font5x7[digit], 5);
}

void display_draw_string(uint8_t x, uint8_t page, uint8_t c){

    if(c < 'A' || c > 'Z')
        return;

    uint8_t addr[] = {
        0x21, x, (uint8_t)(x+4), 
        0x22, page, page
    };

    display_write(DISPLAY_CTRL_CMD, addr, sizeof(addr));
    display_write(DISPLAY_CTRL_DATA, ssd1306_font5x7_chars[c - 'A'], 5); //array starts with 'A'
}

void display_draw_pet(Pet_t *pet){

    //display_clear();

    display_draw_digit(24, 0, pet->bars[0]); //display health digit
    display_draw_digit(48, 0, pet->bars[1]); //display full digit
    display_draw_digit(72, 0, pet->bars[2]); //display clean digit
    display_draw_digit(96, 0, pet->bars[3]); //display happy digit

}

void display_draw_dead(void){
    //display_clear();

    display_draw_string(48, 2, 'D');
    display_draw_string(56, 2, 'E');
    display_draw_string(64, 2, 'A');
    display_draw_string(72, 2, 'D');

}
