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

  map->bricks = (Rectangle *) malloc(max_rect*sizeof(Rectangle));
  if (map->bricks == NULL)
    return false;

  map->portal = (Rectangle *) malloc(max_rect*sizeof(Rectangle));
  if (map->portal == NULL)
    return false;

  strcpy(map->name, "");
  map->max_number_of_bricks = max_rect;
  map->number_of_bricks = 0;
  return true;
}

int load_map(const char *path, Map *map)
{
  assert(path != NULL);

  FILE *file;
  char line[256];
  char arg[256];
  char opcode[256];
  int i;

  file = fopen(path, "r");

  if (file == NULL)
      return -1;

  map->number_of_portals = 0;

  i = 0;
  while (fgets(line, sizeof(line), file))
    {
      sscanf(line, "%255[^ ]%*[ \t]%255[^\n]\n", opcode, arg);
      if(strcmp(opcode, "NAME") == 0)
        {
          strcpy(map->name, arg);
        }
      else if (strcmp(opcode, "BRICK") == 0)
        {
          sscanf(arg, "%c %d,%d %d,%d\n", &map->brick_char,
                 &map->bricks[i].top_left.x,
                 &map->bricks[i].top_left.y,
                 &map->bricks[i].bottom_right.x,
                 &map->bricks[i].bottom_right.y);
          i++;
        }
      else if (strcmp(opcode, "PORTAL") == 0)
        {
          sscanf(arg, "%c %d %d,%d %d,%d\n", &map->portal_char,
                 &map->portal_size,
                 &map->portal[0].top_left.x,
                 &map->portal[0].top_left.y,
                 &map->portal[1].top_left.x,
                 &map->portal[1].top_left.y);

          map->portal[0].bottom_right.x = map->portal[0].top_left.x + map->portal_size;
          map->portal[0].bottom_right.y = map->portal[0].top_left.y + map->portal_size;

          map->portal[1].bottom_right.x = map->portal[1].top_left.x + map->portal_size;
          map->portal[1].bottom_right.y = map->portal[1].top_left.y + map->portal_size;
          map->number_of_portals = 2;
        }
    }
  map->number_of_bricks = i;

  fclose(file);

  print_map(map);
  return i;
}

int get_portal_point(const Map *map, const Point *point, Direction direction, Point *dst_point)
{
  int cur_point_idx, other_point_idx;

  if (map->number_of_bricks != 2)
    return 0;

  if(direction == LEFT || direction == RIGHT)
    {
      if(point->y >= map->portal[0].top_left.y &&
         point->y <= map->portal[0].bottom_right.y)
        {
          other_point_idx = 1;
          cur_point_idx = 0;
        }
      else
        {
          other_point_idx = 0;
          cur_point_idx = 1;
        }
    }
  else
    {
      if(point->x >= map->portal[0].top_left.x &&
         point->x <= map->portal[0].bottom_right.x)
        {
          other_point_idx = 1;
          cur_point_idx = 0;
        }
      else
        {
          other_point_idx = 0;
          cur_point_idx = 1;
        }
    }

      switch(direction)
        {
        case LEFT:
            dst_point->x = map->portal[other_point_idx].top_left.x;
            dst_point->y = map->portal[other_point_idx].top_left.y +
              (point->y - map->portal[cur_point_idx].top_left.y);
            return 1;

        case RIGHT:
            dst_point->x = map->portal[other_point_idx].bottom_right.x;
            dst_point->y = map->portal[other_point_idx].top_left.y +
              (point->y - map->portal[cur_point_idx].top_left.y);
            return 1;

        case UP:
            dst_point->y = map->portal[other_point_idx].top_left.y;
            dst_point->x = map->portal[other_point_idx].top_left.x +
              (point->x - map->portal[cur_point_idx].top_left.x);
            return 1;

        case DOWN:
            dst_point->y = map->portal[other_point_idx].bottom_right.y;
            dst_point->x = map->portal[other_point_idx].top_left.x +
              (point->x - map->portal[cur_point_idx].top_left.x);
            return 1;
        }
  return 0;
}

void print_map(Map *map)
{
  for (int i = 0; i < map->number_of_bricks; i++)
    {
      printf("X1:%d Y1:%d  X2:%d Y2:%d\n", map->bricks[i].top_left.x,
             map->bricks[i].top_left.y,
             map->bricks[i].bottom_right.x,
             map->bricks[i].bottom_right.y);
    }
  printf("\n");
}

void clear_map(Map *map)
{
  free(map->bricks);
}
