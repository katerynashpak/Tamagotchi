#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "pet.h"

void display_init(void);
void display_init_seq(void);
static void display_write(uint8_t control, const uint8_t *buf, uint32_t len);
void display_clear(void);

void display_draw_pet(Pet_t *pet);

void display_set_pixel(uint8_t x, uint8_t y);

#endif //__DISPLAY_H__