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
//#include "ssd1306_fonts.h"

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

//test func for drawing a pixel by coordinate
void display_set_pixel(uint8_t x, uint8_t y){
    // //calc bit
    // //set window
    // //write to byte

    // volatile int d = 0;

    // //check bounds
    // if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    //     return;

    // uint8_t page = y / 8;
    // uint8_t bit = y % 8;

    // uint8_t addr[] = {
    //     0x21, x, x,
    //     0x22, page, page
    // };
    // display_write(DISPLAY_CTRL_CMD, addr, sizeof(addr));
    // for(d = 0; d < 1000; d++);

    // uint8_t pixel = (1U << bit);
    // display_write(DISPLAY_CTRL_DATA, &pixel, 1);
    // for(d = 0; d < 1000; d++);

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


void display_draw_pet(Pet_t *pet){

    //display_clear();

    display_draw_digit(8, 0, pet->bars[0]); //display health digit
    display_draw_digit(32, 0, pet->bars[1]); //display full digit
    display_draw_digit(56, 0, pet->bars[2]); //display clean digit
    display_draw_digit(80, 0, pet->bars[3]); //display happy digit

    // display_draw_digit(8, 0, 0); //display health digit
    // display_draw_digit(32, 0, 1); //display full digit
    // display_draw_digit(56, 0, 2); //display clean digit
    // display_draw_digit(80, 0, 3); //display happy digit
}


