#include "fruit.h"
#include <ncurses.h>
#include <stdlib.h>
#include <assert.h>

bool init_fruits(Fruits *fruits, size_t size)
{
  assert(size>=1);
  fruits->start = (Point *) malloc(size*sizeof(Point));
  if (fruits->start == NULL)
    return false;
  return true;
}

void set_fruit_position(Fruits *fruits, int x, int y)
{
  assert(fruits != NULL);
  fruits->start->x = x;
  fruits->start->y = y;
}

