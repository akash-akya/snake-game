#ifndef DISPLAY_UTIL
#define DISPLAY_UTIL

#define X_MAX 135
#define Y_MAX 37

int set_unit_size (int unit_x, int unit_y);

void print_char_at_point (int pos_y, int pos_x, char ch);

void print_string_at_point (int pos_y, int pos_x, char ch, int length);

void display_delay(long refresh_rate);

void replace_all_char(char src, char dst);

int get_x_max();

int get_y_max();

int get_mid_x();

int get_mid_y();

int get_unit_size();

#endif
