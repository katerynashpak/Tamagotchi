#ifndef __PET_H__
#define __PET_H__


#include <stdint.h>

typedef struct {
    uint8_t bars[4];
    uint32_t days_alive;
} Pet_t;



void pet_tick(void);

#endif // __PET_H__