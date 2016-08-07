#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
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
  size_t max_size;
  Direction last_direction;
} Snake_buffer;

static pthread_mutex_t input_mutex;
volatile sig_atomic_t g_thread_status = 0;

bool snake_init (Snake_buffer *snake_buffer, size_t size)
{
  snake_buffer->start = (Point *) malloc(size*sizeof(Point));
  if (snake_buffer->start == NULL)
    return false;

  for (int i=0; i < (int)size; i++)
    snake_buffer->start[i].x = snake_buffer->start[i].y = 1;

  snake_buffer->head  = snake_buffer->start;
  snake_buffer->max_size = size;
  return true;
}

Point* get_next(const Snake_buffer *snake_buffer, Point *position)
{
  position++;
  if (position == snake_buffer->start + snake_buffer->max_size)
    return snake_buffer->start;
  return position;
}

bool move_by_offset(Snake_buffer *snake_buffer, Direction direction)
{
  Point *next;

  if ((direction == LEFT    && snake_buffer->last_direction == RIGHT)
      || (direction == RIGHT && snake_buffer->last_direction == LEFT)
      || (direction == UP    && snake_buffer->last_direction == DOWN)
      || (direction == DOWN  && snake_buffer->last_direction == UP))
    {
      return false;
    }

  if ((snake_buffer->head->x > 0               && direction == LEFT)
      || (snake_buffer->head->x < get_x_max()   && direction == RIGHT)
      || (snake_buffer->head->y > 0             && direction == UP)
      || (snake_buffer->head->y < get_y_max()   && direction == DOWN))
    {

      snake_buffer->last_direction = direction;

      next = get_next(snake_buffer, snake_buffer->head);
      switch (direction)
        {
        case LEFT:
          next->x = snake_buffer->head->x + -1;
          next->y = snake_buffer->head->y;
          break;

        case RIGHT:
          next->x = snake_buffer->head->x + 1;
          next->y = snake_buffer->head->y;
          break;

        case UP:
          next->x = snake_buffer->head->x;
          next->y = snake_buffer->head->y - 1;
          break;

        case DOWN:
          next->x = snake_buffer->head->x;
          next->y = snake_buffer->head->y + 1;
          break;
        }
      snake_buffer->head = next;
  
      return true;
    }
  else
    {
      return false;
    }
}

void draw_snake (const Snake_buffer *snake_buffer)
{
  Point *point = snake_buffer->head;
  
  for (size_t i = 0; i < snake_buffer->max_size; i++)
    {
      print_point(point->y, point->x, "*");
      point = get_next(snake_buffer, point);
    }
}

void print_buf(Snake_buffer *snake_buffer)
{
  Point *point = snake_buffer->head;
  for (size_t i = 0; i < snake_buffer->max_size; i++)
    {
      point = get_next(snake_buffer, point);
      printf("x:%u y:%u\n", point->x, point->y);
    }
  printf("\n");
}

int is_snake_biting_itself(Snake_buffer *snake_buffer)
{
  Point *head  = snake_buffer->head;
  Point *point = get_next(snake_buffer, head);
    
  for (size_t i = 0; i < snake_buffer->max_size-1; i++)
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

  choice = getch();
  while (choice != 'x')
    {
      pthread_mutex_lock(&input_mutex);
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
      pthread_mutex_unlock(&input_mutex);
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
  g_thread_status = STATUS_STOPPED;
}

void game_loop(Snake_buffer *snake_buffer, long refresh_rate)
{
  while (g_thread_status == STATUS_RUNNING)
    {
      pthread_mutex_lock(&input_mutex);
      move_by_offset(snake_buffer, snake_buffer->last_direction);
      pthread_mutex_unlock(&input_mutex);

      if (is_snake_biting_itself(snake_buffer))
        {
          game_over("Stop biting yourself!");
          continue;
        }
      else if (is_snake_hitting_wall(snake_buffer))
        {
          game_over("Watch your head!");
          continue;
        }

      clear();
      draw_snake((const Snake_buffer *) snake_buffer);
      refresh();

      display_delay(refresh_rate);
    }
}

int main(int argc, char *argv[])
{
  Snake_buffer snake_buffer;
  pthread_t input_reader_thread;  

  if (!snake_init(&snake_buffer, 30))
    {
      printf("Can't allocte memory!!");
      return -1;
    }

  init_window(2);
  
  g_thread_status = STATUS_RUNNING;
  pthread_create(&input_reader_thread, NULL, read_user_input, &snake_buffer);

  snake_buffer.last_direction = RIGHT;
  game_loop(&snake_buffer, 25000);

  pthread_join(input_reader_thread, NULL);
  endwin();
  free(snake_buffer.start);

  return 0;
}
