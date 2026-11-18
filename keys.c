/****************************************************************************
 * @file     keys.c
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/
#include <stdint.h>
#include "keys.h"
#include "tm4c.h"
//#include "pet.h"

/* on-board keys bitmasks for Port D */
#define sleep   (1U << 0)     //PD0
#define feed    (1U << 1)     //PD1
#define bathe   (1U << 2)     //PD2
#define play    (1U << 3)     //PD3


uint32_t keys_read(void){ //keys: 0:sleep, 1:feed, 2:bathe, 3:play
    return GPIOD_AHB->DATA & 0x0F; //bits 0-3, if returns 0 -> pressed
}

