#ifndef PET_H
#define PET_H


#include <stdint.h>

typedef struct {
    uint8_t bars[4];
} Pet_t;



void pet_tick(void);

#endif