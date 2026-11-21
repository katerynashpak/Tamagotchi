/****************************************************************************
 * @file     main.c
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "tm4c.h"
#include "pet.h"
#include "display.h"
#include "keys.h"


/* on-board LEDs bitmasks for Port A */
#define LED_RED     (1U << 2)     //PA2
#define LED_GREEN   (1U << 3)     //PA3
#define LED_BLUE    (1U << 4)     //PA4

/* on-board LEDs bitmasks for Port B */
#define SCL         (1U << 2)     //PB2
#define SDA         (1U << 3)     //PB3

#define NUM_BARS    4
#define MAX_BAR     5

#define SEC_IN_DAY 5 // game day is 60 seconds

volatile uint16_t LIMIT = 50000; //limit of the event duration
volatile uint32_t ms_counter = 0; //volatile since declared in the interrupt
volatile uint32_t sec_counter = 0; // 1s increment, game clock
volatile uint32_t day_sec = 0;

volatile uint8_t eventActive = 0; //0-> false,  1-> true
volatile uint32_t eventTimer = 0; 

volatile uint8_t lastEvent = 0; //last event fired (health--, full--, etc)

typedef enum{
    COLOR_OFF = 0,      //OFF
    COLOR_BLUE = 1,     //HEALTH
    COLOR_ORANGE = 2,   //FULL
    COLOR_PURPLE = 3,   //CLEAN
    COLOR_GREEN = 4,    //JOY
    COLOR_RED = 5,      //DEAD

} Color_t;

void SysTick_Handler(void){
    
    ms_counter++;
    if(ms_counter >= 1000){
        ms_counter = 0;
        sec_counter++; // 1s increment, game clock
        day_sec++; //count seconds in a day and then chech less than SEC_IN_DAY to count and display days alive
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
    }else if(color == COLOR_GREEN){             //joy
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

    uint8_t idx;
    uint8_t is_dead = 0;
    uint8_t i = 0;
    // volatile int n = 0;
    volatile int d = 0;

    RCGCGPIO  |= (1U << 0); //clock, set Bit 0 for Port A
    GPIOHBCTL |= (1U << 0); //hpb, set Bit 0 for Port A

    RCGCGPIO  |= (1U << 1); //clock, set Bit 1 for Port B
    RCGCI2C   |= (1U << 0); //I2C0 module set bit 0
    GPIOHBCTL |= (1U << 1); //hpb, set Bit 1 for Port B

    RCGCGPIO  |= (1U << 3); //clock, set Bit 3 for Port D
    GPIOHBCTL |= (1U << 3); //hpb, set Bit 3 for Port D

    for (d = 0; d < 100; d++);

    //bus clear
    GPIOB_AHB->AFSEL &= ~(SCL | SDA);   // back to GPIO //clear
    GPIOB_AHB->DIR   |= SCL;            // SCL as output
    GPIOB_AHB->ODR   |= SDA;            // SDA open-drain
    GPIOB_AHB->DEN   |= (SCL | SDA);

    for (d = 0; d < 100; d++);

    GPIOD_AHB->DIR   &= ~0X0F;          // PD0-PD3 inputs //clear
    GPIOD_AHB->DEN   |= 0X0F;           // dig enable //set
    GPIOD_AHB->PUR   |= 0X0F;           // internal pull-ups //set

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

    Pet_t pet = {{5,5,5,5}, 1}; //0-health, 1-full, 2-clean, 3-joy

    srand(ms_counter);

    // ---test I2C Display---
    GPIOB_AHB->DATA_Bits[SDA] = SDA; //release SDA
    GPIOB_AHB->DATA_Bits[SCL] = SCL; //SCL high
    display_init();
    display_init_seq();
    //display_fill(0xFF);
    display_clear();
    display_draw_pet(&pet); 
    // ---end test I2C Display---

    while(1){

        // ---test keys---
        static uint32_t prev = 0x0F; //all high
        uint32_t keys = keys_read(); //read which key is pressed
        uint32_t pressed = prev & ~keys; //were 1 and now 0 (key went from released to pressed / falling edge)

        is_dead = 0;
        for(i = 0; i < 4; i++){
            if(pet.bars[i] == 0){
                is_dead = 1;
                break;
            }
        }
                
        if(is_dead){
            set_color(COLOR_RED); //solid
            
            if(pressed){
                //reset game
                pet.bars[0] = pet.bars[1] = pet.bars[2] = pet.bars[3] = MAX_BAR;
                pet.days_alive = 1;
                day_sec = 0;
                sec_counter = 0;
                eventActive = 0;
                is_dead = 0; //false
                display_clear();
                display_draw_pet(&pet);
                display_draw_number(68, 1, pet.days_alive);

            }
            prev = keys;
            continue; //do not count as pet event, skip

        }


        if(!is_dead && day_sec >= SEC_IN_DAY){
            day_sec = 0;
            pet.days_alive++;
            display_draw_number(68, 1, pet.days_alive);
        }


        for(i = 0; i < NUM_BARS; i++){
            if(pressed & (1U << i)) { //bits 0-3
                if(pet.bars[i] < MAX_BAR){
                    pet.bars[i]++;
                    display_draw_pet(&pet); //draw only after event
                    display_event(i); //draw event - sleep, feed, bathe, play

                    uint32_t start = ms_counter;
                    while((ms_counter - start) < 1000){
                        //event frame stays up
                    }
                }
            }
        }
        prev = keys; //update, reset keys back to not pressed

        // ---end test keys---

        //pick random indicator bar to decrement every 30s
        if(sec_counter >= 5){ //set to >= 30 seconds
            sec_counter = 0; //reset
            eventActive = 1; //true
            eventTimer = 0;

            static uint8_t next = 0;
            idx = next;
            next = (next+1) % 4;

            if(pet.bars[idx] > 0){
                pet.bars[idx]--;
                lastEvent = idx; //idx is the index of one of the 4 bars (0-health, ...)
                
                display_draw_pet(&pet); //draw only after event

                if(pet.bars[idx] == 0){
                    display_mood(0);
                }
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
       
        
        if(eventActive){
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

            //draw mood - happy, ok, sad, dead
            display_mood(lowest);

        }    
        
    }

}

