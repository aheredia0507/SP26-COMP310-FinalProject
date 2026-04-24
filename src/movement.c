/**
 *
 * Final Project - Snake Game - Movement Module
 *
 * Implements snake movement logic
 *
 **/

#include "movement.h"

/**
 *
 * Function: move_snake
 *
 * Deliverable:
 * ------------
 * Move the snake one cell in current direction
 *
 * Pseudocode:
 *
 * 1. Calculate new head position based on current direction
 * 2. Shift all body segments back by one position
 * 3. Set new head position
 *
 **/

void move_snake(void) {
    // 1. Calculate new head position based on current direction
    int new_head_x = game.snake_body[0].x;
    int new_head_y = game.snake_body[0].y;

    if(game.current_direction == DIR_RIGHT) new_head_x++;
    if(game.current_direction == DIR_DOWN) new_head_y++;
    if(game.current_direction == DIR_LEFT) new_head_x--;
    if(game.current_direction == DIR_UP) new_head_y--;

    // 2. Shift all body segments back by one position
    for(int i = game.snake_length - 1; i > 0; i--) {
        game.snake_body[i] = game.snake_body[i - 1];
    }

    // 3. Set new head position
    game.snake_body[0].x = new_head_x;
    game.snake_body[0].y = new_head_y;
}

/**
 *
 * Function: handle_direction_input
 *
 * Deliverable:
 * ------------
 * Process arrow key input and update snake direction
 * Prevents 180-degree turns
 *
 * @param scancode - Keyboard scancode from port 0x60
 *
 * Pseudocode:
 *
 * 1. Map scancode to direction
 * 2. Verify new direction is not opposite
 * 3. Update current direction if valid
 *
 **/

void handle_direction_input(uint8_t scancode) {
    // 1. Map scancode to direction
    int new_direction = game.current_direction;
    if(scancode == 0x4D) new_direction = DIR_RIGHT;
    if(scancode == 0x50) new_direction = DIR_DOWN;
    if(scancode == 0x4B) new_direction = DIR_LEFT;
    if(scancode == 0x48) new_direction = DIR_UP;

    // 2-3. Verify new direction is not opposite and update
    if((game.current_direction == DIR_RIGHT && new_direction != DIR_LEFT) ||
       (game.current_direction == DIR_LEFT && new_direction != DIR_RIGHT) ||
       (game.current_direction == DIR_UP && new_direction != DIR_DOWN) ||
       (game.current_direction == DIR_DOWN && new_direction != DIR_UP)) {
        game.current_direction = new_direction;
    }
}
