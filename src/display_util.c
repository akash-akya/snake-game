#include <ncurses.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "display_util.h"

static int g_unit_x, g_unit_y;

inline int get_unit_size() { return g_unit_x; }

int set_unit_size (int unit_x, int unit_y)
{
  assert(unit_x < X_MAX);
  assert(unit_x < Y_MAX);

  g_unit_x = unit_x;
  g_unit_y = unit_y;
  return 0;
}

static void get_string (char *s, int unit, char ch)
{
  int i;

  for (i=0; i<unit; i++)
    s[i] = ch;
  s[i] = '\0';
}

void print_block_point (int pos_y, int pos_x, char ch, int length)
{
  char s[g_unit_x];

  if ((pos_x >= 0 && pos_x <= X_MAX/g_unit_x)
      && (pos_y >= 0 && pos_y <= Y_MAX/g_unit_y))
    {
      get_string(s, length, ch);

      for (int i = 0; i < g_unit_y; i++)
        mvprintw(pos_y*g_unit_y+i, pos_x*g_unit_x, s);
    }
}

void print_char_at_point (int pos_y, int pos_x, char ch)
{
  char s[2] = {ch, '\0'};
  mvprintw(pos_y, pos_x, s);
}

void replace_all_char(char src, char dst)
{
  unsigned long s[2];
  for (int i = 0; i <= Y_MAX/g_unit_y; i++)
    {
      for (int j = 0; j <= X_MAX/g_unit_x; j++)
        if (mvinchnstr(i*g_unit_y, j*g_unit_x, s, 1)
            && s[0] == (unsigned long)src)
          print_block_point(i, j, dst, g_unit_x);
    }
}

inline int get_mid_x() { return X_MAX/2; }

inline int get_mid_y() { return Y_MAX/2; }

inline int get_x_max() { return (X_MAX-1)/g_unit_x; }

inline int get_y_max() { return (Y_MAX-1)/g_unit_y; }

inline void display_delay(long refresh_rate)
{
  usleep(refresh_rate*g_unit_x);
}
