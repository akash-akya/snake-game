#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define X_MAX 135
#define Y_MAX 76
#define REFRESH_RATE 25000

#define STATUS_IDLE 0
#define STATUS_RUNNING 1
#define STATUS_STOPPED 2

typedef enum { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  Point *buf;
  Point *head;
  Point *end;
  Point *start;
  size_t max_size;
  Direction last_direction;
} Snake_buffer;

pthread_mutex_t input_mutex;
volatile sig_atomic_t g_thread_status = 0;

bool snake_init (Snake_buffer *s_buffer, size_t size)
{
  s_buffer->buf = (Point *) malloc(size*sizeof(Point));
  if (s_buffer->buf == NULL)
    return false;

  for (int i=0; i < (int)size; i++)
    s_buffer->buf[i].x = s_buffer->buf[i].y = 0;

  s_buffer->head  = s_buffer->buf;
  s_buffer->start = s_buffer->buf;
  s_buffer->end   = (s_buffer->buf) + size;
  s_buffer->max_size = size;
  return true;
}


Point* get_next(const Snake_buffer *snake_buf, Point *pos)
{
  pos++;
  if (pos == snake_buf->end)
    return snake_buf->start;
  return pos;
}

bool move_by_offset(Snake_buffer *snake_buf, Direction direction)
{
  Point *next;

  if ((direction == LEFT    && snake_buf->last_direction == RIGHT)
      || (direction == RIGHT && snake_buf->last_direction == LEFT)
      || (direction == UP    && snake_buf->last_direction == DOWN)
      || (direction == DOWN  && snake_buf->last_direction == UP))
    {
      return false;
    }

  snake_buf->last_direction = direction;

  if ((snake_buf->head->x <= 0       && direction == LEFT)
      || (snake_buf->head->x >= X_MAX && direction == RIGHT)
      || (snake_buf->head->y <= 0     && direction == UP)
      || (snake_buf->head->y >= Y_MAX && direction == DOWN))
    {
      return false;
    }

  next = get_next(snake_buf, snake_buf->head);
  switch (direction)
    {
    case LEFT:
      next->x = snake_buf->head->x + -1;
      next->y = snake_buf->head->y;
      break;

    case RIGHT:
      next->x = snake_buf->head->x + 1;
      next->y = snake_buf->head->y;
      break;

    case UP:
      next->x = snake_buf->head->x;
      next->y = snake_buf->head->y - 1;
      break;

    case DOWN:
      next->x = snake_buf->head->x;
      next->y = snake_buf->head->y + 1;
      break;
    }  
  snake_buf->head = next;
  
  return true;
}

void draw_snake (const Snake_buffer *snake_buffer)
{
  Point *point = snake_buffer->head;
  
  for (size_t i = 0; i < snake_buffer->max_size-1; i++)
    {
      point = get_next(snake_buffer, point);
      mvprintw(point->y/2, point->x, "*");
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

void init_window()
{
  initscr();
  noecho();
  curs_set(FALSE);
}

int main(int argc, char *argv[])
{
  Snake_buffer snake_buffer;
  pthread_t input_reader_thread;  

  if (!snake_init(&snake_buffer, 30))
    {
      printf("Cant allocte memory!!");
      return -1;
    }

  init_window();
  
  g_thread_status = STATUS_RUNNING;
  pthread_create(&input_reader_thread, NULL, read_user_input, &snake_buffer);

  snake_buffer.last_direction = RIGHT;
  while (g_thread_status == STATUS_RUNNING)
    {
      clear();
      draw_snake((const Snake_buffer *) &snake_buffer);
      refresh();

      pthread_mutex_lock(&input_mutex);
      move_by_offset(&snake_buffer, snake_buffer.last_direction);
      pthread_mutex_unlock(&input_mutex);

      usleep(REFRESH_RATE);
    }

  pthread_join(input_reader_thread, NULL);
  endwin();
  free(snake_buffer.buf);

  return 0;
}
