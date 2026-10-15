#ifndef PET_H
#define PET_H

typedef struct {
    uint32_t happiness;
    uint32_t fullness;
    uint32_t cleanness;
    uint32_t health;
} Pet_t;

void pet_tick(void);