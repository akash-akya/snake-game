#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#include "display_util.h"
#include "main.h"
#include "fruit.h"
#include "map.h"

#define STATUS_PAUSE 0
#define STATUS_RUNNING 1
#define STATUS_STOPPED 2

#define ESC_KEY 27

// ಹಾವಿನ ಆಟ
typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct {
  Point *start;
  Point *head;
  size_t size;
  size_t max_size;
  Direction last_direction;
} Snake_buffer;

bool init_snake (Snake_buffer *snake_buffer, size_t max_size, size_t start_size)
{
  assert(max_size>=1);
  assert(start_size<max_size);

  snake_buffer->start = (Point *) malloc(max_size*sizeof(Point));
  if (snake_buffer->start == NULL)
    return false;

  for (int i=0; i < (int)max_size; i++)
    snake_buffer->start[i].x = snake_buffer->start[i].y = 1;

  snake_buffer->head     = snake_buffer->start;
  snake_buffer->max_size = max_size;
  snake_buffer->size     = start_size;
  return true;
}

Point* get_next(const Snake_buffer *snake_buffer, Point *position)
{
  position++;
  if (position == snake_buffer->start + snake_buffer->size)
    return snake_buffer->start;
  return position;
}

void add_fruit(Snake_buffer *snake_buffer, Fruits *fruits)
{
  int x,y;
  int is_point_unique = 0;

  while (!is_point_unique)
    {
      x = 1+rand()%(get_x_max()-1);
      y = 1+rand()%(get_y_max()-1);
      is_point_unique = 1;

      Point *point = get_next(snake_buffer, snake_buffer->head);

      for (size_t i = 0; i < snake_buffer->size-1; i++)
        {
          if (point->x == x && point->y == y)
            {
              is_point_unique = 0;
              break;
            }
        }
    }
  fruits->start->x = x;
  fruits->start->y = y;
}

int for_each_point(const Snake_buffer *snake_buffer, int (*operation)(const Point *))
{
  assert(snake_buffer != NULL);
  int ret;
  Point *point = snake_buffer->head;
  for (size_t i = 0; i < snake_buffer->size; i++)
    {
      ret = operation(point);
      if (ret != 0)
        return ret;
      point = get_next(snake_buffer, point);
    }
  return 0;
}

bool move_by_offset(Snake_buffer *snake_buffer, Direction direction)
{
  // Snake should not move backwards
  // ಹಾವಿನ ಹಿಂಬದಿಯ ಚಲನೆ ನಿಷೇದಿಸಿದೆ
  if ((direction == LEFT    && snake_buffer->last_direction == RIGHT)
      || (direction == RIGHT && snake_buffer->last_direction == LEFT)
      || (direction == UP    && snake_buffer->last_direction == DOWN)
      || (direction == DOWN  && snake_buffer->last_direction == UP))
    {
      return false;
    }

  Point *next = get_next(snake_buffer, snake_buffer->head);
  next->x = snake_buffer->head->x;
  next->y = snake_buffer->head->y;

  switch (direction)
    {
    case LEFT:  next->x -= 1;
      break;
    case RIGHT: next->x += 1;
      break;
    case UP:    next->y -= 1;
      break;
    case DOWN:  next->y += 1;
      break;
    }
  snake_buffer->head = next;

  // Save current direction as last-direction
  snake_buffer->last_direction = direction;

  return true;
}

int draw_point(const Point *p)
{
  print_block_point(p->y, p->x, '*', get_unit_size());
  return 0;
}

// for debuguing
int debug_print_point(const Point *p) { printf("x:%u y:%u\n", p->x, p->y); return 0;}

int is_snake_biting_itself(Snake_buffer *snake_buffer)
{
  Point *head  = snake_buffer->head;
  Point *point = get_next(snake_buffer, head);

  for (size_t i = 0; i < snake_buffer->size-1; i++)
    {
      if (point->x == head->x && point->y == head->y)
        return 1;
      point = get_next(snake_buffer, point);
    }
  return 0;
}

void init_window(int block_size)
{
  initscr();
  noecho();
  curs_set(FALSE);
  set_unit_size(block_size, block_size);
}

int is_snake_hitting_wall(Snake_buffer *snake_buffer)
{
  if ((snake_buffer->head->x <= 0)
      || (snake_buffer->head->x >= get_x_max())
      || (snake_buffer->head->y <= 0)
      || (snake_buffer->head->y >= get_y_max()))
    {
      return 1;
    }
  return 0;
}

void game_over (const char *msg)
{
  const int rate = 7;
  static int state = 0;
  unsigned int len_msg = strlen(msg);

  mvprintw(get_mid_y()-1, get_mid_x()-len_msg/2, msg);
  mvprintw(get_mid_y(), get_mid_x()-5, "GAME-OVER");
  refresh();

  int t = state < rate ? 0 : 1;
  replace_all_char("*."[t], ".*"[t]);
  state = state > rate<<1 ? 0 : state+1;
}

void display_fruits(const Fruits *fruits)
{
  assert(fruits != NULL);
  print_block_point(fruits->start->y, fruits->start->x, '@', get_unit_size());
}

void show_map_name(const char *name)
{
  mvprintw(0, 136-strlen(name), name);
}

void display_map(const Map *map)
{
  show_map_name(map->name);
  for (int i = 0; i < map->number_of_rect; i++)
    {
      print_rect(map->rect[i].top_left.y,
                 map->rect[i].top_left.x,
                 map->rect[i].bottom_right.y-map->rect[i].top_left.y,
                 map->rect[i].bottom_right.x-map->rect[i].top_left.x,
                 get_unit_size(),
                 '8');
    }
}

void display_score(int score)
{
  mvprintw(0, 1, "Score: %d", score);
}

int update_game_status(Snake_buffer *snake_buffer, Fruits *fruits)
{
  Point *point = get_next(snake_buffer, snake_buffer->head);

  for (size_t i = 0; i < snake_buffer->size-1; i++)
    {
      if (point->x == fruits->start->x && point->y == fruits->start->y)
        {
          add_fruit(snake_buffer, fruits);
          return 1;
        }
      point = get_next(snake_buffer, point);
    }
  return 0;
}

void increase_snake_size (Snake_buffer *snake_buffer)
{
  assert(snake_buffer->size < snake_buffer->max_size);
  Point *point = NULL;

  for (int i=0 ; i < 3; i++) {
    point = snake_buffer->start+snake_buffer->size;
    point->x = -1;
    point->y = -1;
    snake_buffer->size += 1;
  }
}

void display_status (int status)
{
  switch (status)
    {
    case STATUS_PAUSE: mvprintw(0, 11, "PAUSED");
      break;
    }
}

void draw_buffer_on_screen(const Snake_buffer *snake_buffer, const Fruits *fruits, const Map *map, const unsigned int score)
{
  clear();
  (void)for_each_point(snake_buffer, draw_point);
  display_map(map);
  display_fruits(fruits);
  display_score(score);
  refresh();
}

void game_loop(Snake_buffer *snake_buffer, Fruits *fruits, Map *map, long refresh_rate)
{
  const int RUNNING = 0;
  const int GAME_OVER = 1;
  const int PAUSED = 2;

  unsigned int score = 0;
  int ch = 0;
  int game_state = RUNNING;

  nodelay(stdscr, TRUE);
  keypad(stdscr,TRUE);  /* allow keypad keys to be used */

  while ((ch = getch()) != 'x')
    {
      if (game_state == RUNNING)
        {
          switch(ch)
            {
            case KEY_LEFT:
            case 'a': move_by_offset(snake_buffer, LEFT);
              break;

            case KEY_DOWN:
            case 's': move_by_offset(snake_buffer, DOWN);
              break;

            case KEY_UP:
            case 'w': move_by_offset(snake_buffer, UP);
              break;

            case KEY_RIGHT:
            case 'd': move_by_offset(snake_buffer, RIGHT);
              break;

            case 'p': game_state = PAUSED;
              break;

            default:
              move_by_offset(snake_buffer, snake_buffer->last_direction);
              break;
            }

          if (update_game_status(snake_buffer, fruits))
            {
              score++;
              increase_snake_size(snake_buffer);
            }

          draw_buffer_on_screen(snake_buffer, fruits, map, score);
        }
      else if (game_state == PAUSED)
        {
          if (ch == 'r')
            game_state = RUNNING;
        }

      if (is_snake_biting_itself(snake_buffer))
        {
          game_state = GAME_OVER;
          game_over("Stop biting yourself!");
        }
      else if (is_snake_hitting_wall(snake_buffer))
        {
          game_state = GAME_OVER;
          game_over("Watch your head!");
        }

      display_delay(refresh_rate);
    }
}

int main(int argc, char *argv[])
{
  char *map_file_name = NULL;
  Snake_buffer snake_buffer;
  Fruits fruits;
  Map map;

  if (argc>0)
    map_file_name = argv[1];

  if (!init_snake(&snake_buffer, 60, 3))
    {
      printf("Can't allocte memory for snake!!");
      return -1;
    }

  if (!init_fruits(&fruits, 1))
    {
      printf("Can't allocte memory for fruits!!");
      return -1;
    }

  if (!init_map(&map, 30))
    {
      printf("Can't allocte memory for map!!");
      return -1;
    }

  // Randomize seed
  srand(time(NULL));

  init_window(1);

  snake_buffer.last_direction = RIGHT;
  add_fruit(&snake_buffer, &fruits);

  if (map_file_name != NULL)
    if (load_map(map_file_name, &map) == -1)
      return -1;

  print_map(&map);
  game_loop(&snake_buffer, &fruits, &map, 50000);

  endwin();

  free(snake_buffer.start);
  clear_fruits(&fruits);
  clear_map(&map);

  return 0;
}
