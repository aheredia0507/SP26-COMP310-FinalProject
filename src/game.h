/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game - Game Module
 *
 * Game state management and initialization
 *
 **/

#ifndef GAME_H
#define GAME_H

#include "snake.h"

// Game initialization and control
void init_game(void);
void init_snake(void);
void game_loop(void);
void read_keyboard_input(void);
void setup_test_mode(void);

// Random number generation
void init_random(void);
void seed_random_from_pit(void);
int random_int(void);
int random_range(int min, int max);

// Collision detection
int check_wall_collision(void);
int check_self_collision(void);
int check_win_condition(void);

#endif
