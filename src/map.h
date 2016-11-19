#ifndef _H_MAP
#define _H_MAP

#include <ncurses.h>
#include "main.h"

typedef struct {
  Point top_left;
  Point bottom_right;
} Rectangle;

typedef struct {
  Rectangle *bricks;
  Rectangle *portal;
  char name[256];
  char brick_char;
  char portal_char;
  int portal_size;
  int number_of_bricks;
  int number_of_portals;
  int max_number_of_bricks;
} Map;

bool init_map(Map *map, int max_rect);

int load_map(const char *path, Map *map, const char brick_char, const char portal_char);

void print_map(Map *map);

void clear_map(Map *map);

int get_portal_point(const Map *map, const Point *point, Direction direction, Point *dst_point);

#endif
