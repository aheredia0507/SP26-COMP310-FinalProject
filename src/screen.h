/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game - Screen Header
 *
 * Declares all screen rendering functions
 *
 **/

#ifndef SCREEN_H
#define SCREEN_H

#include "snake.h"

// Drawing functions
void draw_char(char c, int x, int y, int scale, int color);
void draw_text(char *text, int x, int y, int scale, int color);
void draw_cell(int grid_x, int grid_y, int color);
void draw_big_cell(int grid_x, int grid_y, int color);
void draw_segment(int grid_x, int grid_y, int color);
void draw_apple(int grid_x, int grid_y);

// Utility functions
int get_background_color(int x, int y);
void clear_screen(void);
void clear_full_screen(int color);
void delay(unsigned int iterations);
void draw_pixel_rect(int x, int y, int width, int height, int color);

// Screen state functions
void draw_title_screen(void);
void draw_how_to_play_screen(void);
void draw_game_over_screen(void);
void draw_you_win_screen(void);
void draw_initial_game_state(void);

// Game rendering
void render_game(void);
void reset_render_flag(void);

#endif
