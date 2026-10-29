#include "pet.h"
#include <stdint.h>


struct Pet_t {
  uint8_t bars[4]; // 0-healthy, 1-full, 2-clean, 3-happy
};

void pet_tick(void) {}
