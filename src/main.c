#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "display_util.h"

#define STATUS_IDLE 0
#define STATUS_RUNNING 1
#define STATUS_STOPPED 2

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  Point *start;
  Point *head;
  size_t size;
  size_t max_size;
  Direction last_direction;
} Snake_buffer;

typedef struct {
  Point *start;
} Fruits;

static pthread_mutex_t buffer_access_mutex;
static volatile sig_atomic_t g_thread_status = 0;

bool snake_init (Snake_buffer *snake_buffer, size_t size)
{
  assert(size>=1);
  snake_buffer->start = (Point *) malloc(size*sizeof(Point));
  if (snake_buffer->start == NULL)
    return false;

  for (int i=0; i < (int)size; i++)
    snake_buffer->start[i].x = snake_buffer->start[i].y = 1;

  snake_buffer->head  = snake_buffer->start;
  snake_buffer->max_size = size;
  snake_buffer->size = 0;
  return true;
}

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

int for_each_point(Snake_buffer *snake_buffer, int (*operation)(const Point *))
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
  if ((direction == LEFT    && snake_buffer->last_direction == RIGHT)
      || (direction == RIGHT && snake_buffer->last_direction == LEFT)
      || (direction == UP    && snake_buffer->last_direction == DOWN)
      || (direction == DOWN  && snake_buffer->last_direction == UP))
    {
      return false;
    }

  pthread_mutex_lock(&buffer_access_mutex);

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

  pthread_mutex_unlock(&buffer_access_mutex);
  return true;
}

int draw_point(const Point *p)
{
  print_string_at_point(p->y, p->x, '*', get_unit_size());
  return 0;
}

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

void *read_user_input (void *arg)
{
  char choice;
  Snake_buffer *snake_buffer = (Snake_buffer *)arg;

  assert(snake_buffer != NULL);

  choice = getch();
  while (choice != 'x')
    {
      switch(choice)
        {
        case 'a': move_by_offset(snake_buffer, LEFT);
          break;
        case 's': move_by_offset(snake_buffer, DOWN);
          break;
        case 'w': move_by_offset(snake_buffer, UP);
          break;
        case 'd': move_by_offset(snake_buffer, RIGHT);
          break;
        }
      choice = getch();
    }
  g_thread_status = STATUS_STOPPED;
  return NULL;
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
  unsigned int len_msg = strlen(msg);
  mvprintw(get_mid_y()-1, get_mid_x()-len_msg/2, msg);
  mvprintw(get_mid_y(), get_mid_x()-5, "GAME-OVER");
  refresh();

  int t = 0;
  while (g_thread_status == STATUS_RUNNING)
    {
      replace_all_char("*."[t], ".*"[t]);
      t = (t+1)%2;
      refresh();
      display_delay(150000);
    }
}

void display_fruits(Fruits *fruits)
{
  assert(fruits != NULL);
  print_string_at_point(fruits->start->y, fruits->start->x, '$', get_unit_size());
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
  Point *point = snake_buffer->start+snake_buffer->size;

  point->x = -1;
  point->y = -1;

  snake_buffer->size += 1;
}

void game_loop(Snake_buffer *snake_buffer, Fruits *fruits, long refresh_rate)
{
  while (g_thread_status == STATUS_RUNNING)
    {
      move_by_offset(snake_buffer, snake_buffer->last_direction);

      clear();
      (void)for_each_point(snake_buffer, draw_point);
      display_fruits(fruits);
      refresh();

      if (update_game_status(snake_buffer, fruits))
        increase_snake_size(snake_buffer);

      if (is_snake_biting_itself(snake_buffer))
        game_over("Stop biting yourself!");
      else if (is_snake_hitting_wall(snake_buffer))
        game_over("Watch your head!");
      else
        display_delay(refresh_rate);
    }
}

int main(int argc, char *argv[])
{
  Snake_buffer snake_buffer;
  Fruits fruits;

  pthread_t input_reader_thread;

  if (!snake_init(&snake_buffer, 30))
    {
      printf("Can't allocte memory for snake!!");
      return -1;
    }

  if (!init_fruits(&fruits, 1))
    {
      printf("Can't allocte memory for fruits!!");
      return -1;
    }

  init_window(2);

  g_thread_status = STATUS_RUNNING;
  pthread_create(&input_reader_thread, NULL, read_user_input, &snake_buffer);

  snake_buffer.size = 3;
  snake_buffer.last_direction = RIGHT;
  add_fruit(&snake_buffer, &fruits);
  game_loop(&snake_buffer, &fruits, 55000);

  pthread_join(input_reader_thread, NULL);
  endwin();
  free(snake_buffer.start);

  return 0;
}
