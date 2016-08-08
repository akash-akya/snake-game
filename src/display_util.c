#include <ncurses.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "display_util.h"

static int g_unit_x, g_unit_y;

int set_unit_size (int unit_x, int unit_y)
{
  assert(unit_x < X_MAX);
  assert(unit_x < Y_MAX);

  g_unit_x = unit_x;
  g_unit_y = unit_y;
  return 0;
}

int get_mid_x ()
{
  return X_MAX/2;
}

int get_mid_y ()
{
  return Y_MAX/2;
}

void get_string (char *s, int unit, char ch)
{
  int i;
  
  for (i=0; i<unit; i++)
    s[i] = ch;
  s[i] = '\0';
}

void print_point (int pos_y, int pos_x, char *ch)
{
  char s[g_unit_x];
  
  assert(ch != NULL);
  assert(pos_x >= 0 && pos_x <= X_MAX/g_unit_x);
  assert(pos_y >= 0 && pos_y <= Y_MAX/g_unit_y);

  get_string(s, g_unit_x, '*');

  for (int i = 0; i < g_unit_y; i++)
    mvprintw(pos_y*g_unit_y+i, pos_x*g_unit_x, s);
}

inline int get_x_max()
{
  return  (X_MAX-1)/g_unit_x;
}

inline int get_y_max()
{
  return  (Y_MAX-1)/g_unit_y;
}

inline void display_delay(long refresh_rate)
{
  usleep(refresh_rate*g_unit_x);
}
