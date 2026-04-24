/**
 *
 * Final Project - Snake Game - Screen Header
 *
 * Declares all screen rendering functions
 *
 **/

#ifndef SCREEN_H
#define SCREEN_H

#include "snake.h"

// Core drawing utilities
void draw_char(char c, int x, int y, int scale, int color);
void draw_text(char *text, int x, int y, int scale, int color);
void draw_int(int n, int x, int y, int scale, int color);

// Cell and segment drawing
void draw_cell(int pixel_x, int pixel_y, int color);
void draw_big_cell(int grid_x, int grid_y, int color);
void draw_segment(int grid_x, int grid_y, int color);

// Fruit sprite drawing
void draw_apple(int grid_x, int grid_y);
void draw_apple_at_pixel(int pixel_x, int pixel_y);

// Screen management
void clear_screen(void);
void clear_full_screen(int color);
void reset_render_flag(void);

// Game rendering
void draw_initial_game_state(void);
void render_game(void);

// Menu screens
void draw_title_screen(void);
void draw_how_to_play_screen(void);
void draw_game_over_screen(void);
void draw_you_win_screen(void);

// Delay utility
void delay(unsigned int n);

#endif
