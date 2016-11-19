#ifndef _H_MAP
#define _H_MAP

#include <ncurses.h>
#include "main.h"

typedef struct {
  Point top_left;
  Point bottom_right;
} Rectangle;

typedef struct {
  Rectangle *rect;
  char name[256];
  char brick_char;
  int number_of_rect;
  int max_number_of_rect;
} Map;

bool init_map(Map *map, int max_rect);

int load_map(const char *path, Map *map, const char brick_char);

void print_map(Map *map);

void clear_map(Map *map);

#endif
