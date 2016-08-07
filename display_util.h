#ifndef DISPLAY_UTIL
#define DISPLAY_UTIL

#define X_MAX 135
#define Y_MAX 37

int set_unit_size (int unit_x, int unit_y);
void print_point (int pos_y, int pos_x, char *ch);
int get_x_max();
int get_y_max();
void display_delay(long refresh_rate);

#endif
