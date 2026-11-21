/****************************************************************************
 * @file     pet.c
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/


#include "pet.h"
#include <stdint.h>


struct Pet_t {
  uint8_t bars[4]; // 0-healthy, 1-full, 2-clean, 3-joy
  uint32_t days_alive;
};

void pet_tick(void) {}
