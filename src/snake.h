/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game
 *
 * Main header file with shared structures and constants
 *
 **/

#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

// Grid dimensions
#define GRID_WIDTH  12
#define GRID_HEIGHT 9
#define CELL_SIZE   30

// Snake/fruit render size (2x2 tiles = 60x60 pixels)
#define SEGMENT_SIZE 2

// Screen dimensions and margins
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define GAME_AREA_WIDTH  (GRID_WIDTH * SEGMENT_SIZE * CELL_SIZE)
#define GAME_AREA_HEIGHT (GRID_HEIGHT * SEGMENT_SIZE * CELL_SIZE)

// Centering
#define OFFSET_X ((SCREEN_WIDTH - GAME_AREA_WIDTH) / 2)
#define OFFSET_Y ((SCREEN_HEIGHT - GAME_AREA_HEIGHT) / 2)

// Maximum snake length
#define MAX_SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)

// Direction constants
#define DIR_RIGHT 0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_UP    3

// Game state constants
#define STATE_TITLE_SCREEN 0
#define STATE_HOW_TO_PLAY  1
#define STATE_PLAYING      2
#define STATE_GAME_OVER    3
#define STATE_YOU_WIN      4

// Cell content types for frame buffer
#define CELL_EMPTY      0
#define CELL_SNAKE_HEAD 1
#define CELL_SNAKE_BODY 2
#define CELL_FRUIT      3

// Color constants
#define COLOR_BACKGROUND_DARK  0x1a1a1a
#define COLOR_BACKGROUND_LIGHT 0x2d2d2d
#define COLOR_SNAKE_HEAD       0x006400
#define COLOR_SNAKE_BODY       0x00AA00
#define COLOR_FRUIT            0xFF0000
#define COLOR_TEXT             0xFFFFFF
#define COLOR_BLACK            0x000000
#define COLOR_YELLOW           0xFFFF00

// Position structure
struct Position {
    int x;
    int y;
};

// Game state structure
struct GameState {
    struct Position snake_body[MAX_SNAKE_LENGTH];
    int snake_length;
    int current_direction;
    struct Position fruit;
    int game_state;
    int score;
    unsigned int random_seed;
    int fruit_active;
};

// Global game state (declared in game.c)
extern struct GameState game;

// External functions from kernel_main.c
extern void drawPixel(int x, int y, int color);
extern uint8_t inb(uint16_t port);

#endif
