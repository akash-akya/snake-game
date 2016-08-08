#ifndef _H_FRUIT
#define _H_FRUIT

#include <ncurses.h>
#include "main.h"

typedef struct {
  Point *start;
} Fruits;

bool init_fruits(Fruits *fruits, size_t size);

void set_fruit_position(Fruits *fruits, int x, int y);

#endif
