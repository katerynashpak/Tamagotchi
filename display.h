#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "pet.h"

void display_init(void);
void display_init_seq(void);
void display_write(uint8_t control, const uint8_t *buf, uint32_t len);
void display_fill(uint8_t value);
void display_clear(void);

void display_set_pixel(uint8_t x, uint8_t y);

void display_draw_digit(uint8_t x, uint8_t page, uint8_t digit);

void display_draw_number(uint8_t x, uint8_t page, uint16_t num);

void display_draw_string(uint8_t x, uint8_t page, uint8_t c);

void display_draw_pet(Pet_t *pet);

void display_draw_dead(void);

void display_mood(uint8_t lowest);

void display_event(uint8_t key);

#endif //__DISPLAY_H__