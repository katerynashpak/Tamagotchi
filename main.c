/****************************************************************************
 * @file     main.c
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "tm4c.h"
//#include "tm4c123gh6pm.h"
#include "pet.h"


/* on-board LEDs bitmasks for Port A */
#define LED_RED     (1U << 2)     //PA2
#define LED_GREEN   (1U << 3)     //PA3
#define LED_BLUE    (1U << 4)     //PA4

/* on-board LEDs bitmasks for Port B */
#define SCL         (1U << 2)     //PB2
#define SDA         (1U << 3)     //PB3


volatile uint16_t LIMIT = 50000; //limit of the event duration
volatile uint32_t ms_counter = 0; //volatile since declared in the interrupt
volatile uint32_t sec_counter = 0; // 1s increment, game clock

volatile uint8_t eventActive = 0; //0-> false,  1-> true
volatile uint32_t eventTimer = 0; 

volatile uint8_t lastEvent = 0; //last event fired (health--, full--, etc)

typedef enum{
    COLOR_OFF = 0,      //OFF
    COLOR_BLUE = 1,     //HEALTH
    COLOR_ORANGE = 2,   //FULL
    COLOR_PURPLE = 3,   //CLEAN
    COLOR_GREEN = 4,    //HAPPY
    COLOR_RED = 5,      //DEAD

} Color_t;

void SysTick_Handler(void){
    //pet_tick();
    
    ms_counter++;
    if(ms_counter >= 1000){
        ms_counter = 0;
        sec_counter++; // 1s increment, game clock
    }
    
}

void set_color(uint8_t color){
    
    GPIOA_AHB->DATA = 0; //OFF

    if(color == COLOR_BLUE){                    //health
        GPIOA_AHB->DATA = LED_BLUE;
    }else if(color == COLOR_ORANGE){            //full
        GPIOA_AHB->DATA = LED_RED | LED_GREEN;
    }else if(color == COLOR_PURPLE){            //clean
        GPIOA_AHB->DATA = LED_BLUE | LED_RED;
    }else if(color == COLOR_GREEN){             //happy
        GPIOA_AHB->DATA = LED_GREEN;
    }else if(color == COLOR_RED){               //dead
        GPIOA_AHB->DATA = LED_RED;
    }

    //OFF (handled in main)
}


Color_t color_from_event(uint8_t event){
    if(event == 0) return COLOR_BLUE;
    if(event == 1) return COLOR_ORANGE;
    if(event == 2) return COLOR_PURPLE;
    if(event == 3) return COLOR_GREEN;

    if(event == 5) return COLOR_RED;
    return COLOR_OFF;
}


int main(void){
    
    //
    // Hello!
    //
    //UARTprintf("Hello, world!\n");

    uint8_t idx;
    uint8_t is_dead = 0;
    uint8_t i = 0;
    volatile int n = 0;
    volatile int d = 0;

    RCGCGPIO  |= (1U << 0); //clock, set Bit 0 for Port A
    GPIOHBCTL |= (1U << 0); //hpb, set Bit 0 for Port A

    RCGCGPIO  |= (1U << 1); //clock, set Bit 1 for Port B
    RCGCI2C   |= (1U << 0); //I2C0 module set bit 0
    GPIOHBCTL |= (1U << 1); //hpb, set Bit 1 for Port B

    //bus cleae
    GPIOB_AHB->AFSEL &= ~(SCL | SDA);   // back to GPIO
    GPIOB_AHB->DIR   |= SCL;            // SCL as output
    GPIOB_AHB->ODR   |= SDA;            // SDA open-drain
    GPIOB_AHB->DEN   |= (SCL | SDA);

    for (i = 0; i < 9; i++) {
        GPIOB_AHB->DATA_Bits[SCL] = 0;
        for (d = 0; d < 100; d++);
        GPIOB_AHB->DATA_Bits[SCL] = SCL;
        for (d = 0; d < 100; d++);
    }
    //end bus clear


    //PA - RGB LED
    GPIOA_AHB->DIR |= (LED_RED | LED_GREEN | LED_BLUE); //PA2, PA3, PA4
    GPIOA_AHB->DEN |= (LED_RED | LED_GREEN | LED_BLUE); //connect to digital pads

    SysTick->LOAD = 16000 - 1; // 16 MHz - 1000 = 1ms
    SysTick->VAL = 0; //clear current count
    SysTick->CTRL = 0x7; //enable & interrupt & clock -> 111 is 7

    

    //PB - I2C Display
    GPIOB_AHB->AFSEL |= (SCL | SDA); //PB2, PB3
    GPIOB_AHB->PCTL  |= (3U << 8) | (3U << 12); //both 3 bits, PMC2 bit 8, PMC3 bit 12
    GPIOB_AHB->DEN   |= (SCL | SDA);
    GPIOB_AHB->ODR   |= SDA; //open-drain operation

    I2C0->MCR = 0x10;   //master mode
    I2C0->MTPR = 24;     // for ~100 kHz at 16 MHz

    Pet_t pet = {{5,5,5,5}}; //0-health, 1-full, 2-clean, 3-happy

    srand(ms_counter);


    //test LED
    // GPIOA_AHB->DATA_Bits[LED_RED] = LED_RED;
    // GPIOA_AHB->DATA_Bits[LED_GREEN] = LED_GREEN;
    // GPIOA_AHB->DATA_Bits[LED_BLUE] = LED_BLUE;
    //end test LED


    //test I2C Display
    //command mode

    //0xAE - display off
    I2C0->MSA = (0x3C << 1);    //adr + write bit 0
    I2C0->MDR = 0x00;           //control byte
    I2C0->MCS = 0x03;           //start + run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xAE;           //display on //A5 all on test
    I2C0->MCS = 0x05;           //start + stop
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
        GPIOA_AHB->DATA = LED_RED;
        while (1);
    }

    //0xD5 0x80 clock
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xD5;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x80;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xA8 0x3F multiplex
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xA8;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x3F;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xD3 0x00 offset
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xD3;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0x40 start line
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x40;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0x8D 0x14 charge pump on
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x8D;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x14;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0x20 0x00 horiz adr
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x20;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xA1 segment remap
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xA1;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xC8 COM scan
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xC8;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xDA 0x12 COM pins
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xDA;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x12;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0x81 0xCF contrast
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x81;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xCF;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xD9 0xF1 pre-charge
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xD9;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xF1;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xDB 0x40 VCOM
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xDB;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0x40;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
        GPIOA_AHB->DATA = LED_RED;
        while (1);
    }

    //0xA4 resume RAM
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xA4;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xA6 normal display
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xA6;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
    GPIOA_AHB->DATA = LED_RED;
    while (1);
}

    //0xAF segment remap
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);
    I2C0->MDR = 0xAF;           //display on
    I2C0->MCS = 0x05;           //start + stop
    while(I2C0->MCS & 0x01);
    if (I2C0->MCS & 0x02) {
        GPIOA_AHB->DATA = LED_RED;
        while (1);
    }



    //col adr and page adr

    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0x21;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x01;          
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0x7F;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);

    if (I2C0->MCS & 0x02) {
        GPIOA_AHB->DATA = LED_RED;
        while (1);
    }


    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0x22;           
    I2C0->MCS = 0x01;           //run
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x01;          
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0x07;           
    I2C0->MCS = 0x05;          
    while(I2C0->MCS & 0x01);

    if (I2C0->MCS & 0x02) {
        GPIOA_AHB->DATA = LED_RED;
        while (1);
    }



    //data mode
    I2C0->MSA = (0x3C << 1);
    I2C0->MDR = 0x40;           //control byte = data
    I2C0->MCS = 0x03;           //start + run
    while(I2C0->MCS & 0x01);

    for(n = 0; n < 102400; n++){
        I2C0->MDR = 0xFF;           
        I2C0->MCS = 0x01;       //run
        while(I2C0->MCS & 0x01);
    }
    I2C0->MCS = 0x04;           //stop
    while(I2C0->MCS & 0x01);


    //last test:
    I2C0->MSA = (0x3C << 1);    
    I2C0->MDR = 0x00;           
    I2C0->MCS = 0x03;           
    while(I2C0->MCS & 0x01);

    I2C0->MDR = 0xA5;           
    I2C0->MCS = 0x05;           
    while(I2C0->MCS & 0x01);


    if(I2C0->MCS & 0x02){ //if error -> red led on
        GPIOA_AHB->DATA_Bits[LED_RED] = LED_RED;
        while(1);
    }

    //end test I2C Display


    while(1){
       // led
        //GPIOA_AHB->DATA_Bits[LED_GREEN] = LED_GREEN; //happy

        //pick random indicator bar to decrement every 30s
        if(sec_counter >= 5){ //set to >= 30
            sec_counter = 0; //reset
            eventActive = 1; //true
            eventTimer = 0;

            //idx = rand() % 4; //pick from bars array idx 0-3

            static uint8_t next = 0;
            idx = next;
            next = (next+1) % 4;

            if(pet.bars[idx] > 0){
                pet.bars[idx]--;
                lastEvent = idx; //idx is the index of one of the 4 bars (0-health, ...)
            }
        }

        //check if dead
        is_dead = 0;
        for(i = 0; i < 4; i++){
            if(pet.bars[i] == 0){
                is_dead = 1; //true
                break; //out of the flashing / event loop
            }
        }

        //check lowest bar for state assignment
        uint8_t lowest = pet.bars[0];
        uint8_t lowest_idx = 0; //health

        for(i = 1; i < 4; i++){
            if(pet.bars[i] < lowest){
                lowest = pet.bars[i];
                lowest_idx = i;
            }
        }
       
                
        //LED
        if(is_dead){
            set_color(COLOR_RED); //solid
        }else if(eventActive){
            if((eventTimer / 20000) % 2 == 0){ //ON-OFF blinking
                set_color(color_from_event(lastEvent)); //color depends on which of the 4 bars decreased (type of event fired)
            }else{
                set_color(COLOR_OFF);
            }
            
            
            if(++eventTimer > LIMIT){ //# of iterations
                eventActive = 0; //false
                GPIOA_AHB->DATA = 0; //turn all off
            }
        }else{
            //state -> solid color
            
            //check lowest
            if(lowest_idx == 0){
                set_color(COLOR_BLUE);
            }else if(lowest_idx == 1){
                set_color(COLOR_ORANGE);
            }else if(lowest_idx == 2){
                set_color(COLOR_PURPLE);
            }else if(lowest_idx == 3){
                set_color(COLOR_GREEN);
            }

        }          
    }

}

