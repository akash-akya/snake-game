#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ncurses.h>

#include "map.h"

bool init_map(Map *map, int max_rect)
{
  assert(max_rect>=1);

  map->rect = (Rectangle *) malloc(max_rect*sizeof(Rectangle));
  if (map->rect == NULL)
    return false;

  strcpy(map->name, "");
  map->max_number_of_rect = max_rect;
  map->number_of_rect = 0;
  return true;
}

int load_map(const char *path, Map *map)
{
  assert(path != NULL);

  FILE *file;
  char line[256];
  int i;

  file = fopen(path, "r");

  if (file == NULL)
      return -1;

  fgets(map->name, sizeof(line), file);

  i = 0;
  while (fgets(line, sizeof(line), file) && i < map->max_number_of_rect)
    {
      sscanf(line, "%d,%d %d,%d\n", &map->rect[i].top_left.x,
             &map->rect[i].top_left.y,
             &map->rect[i].bottom_right.x,
             &map->rect[i].bottom_right.y);
      i++;
    }
  map->number_of_rect = i;

  fclose(file);
  return i;
}

void print_map(Map *map)
{
  for (int i = 0; i < map->number_of_rect; i++)
    {
      printf("X1:%d Y1:%d  X2:%d Y2:%d\n", map->rect[i].top_left.x,
             map->rect[i].top_left.y,
             map->rect[i].bottom_right.x,
             map->rect[i].bottom_right.y);
    }
  printf("\n");
}

void clear_map(Map *map)
{
  free(map->rect);
}
