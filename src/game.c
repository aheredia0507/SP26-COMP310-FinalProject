/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game - Game Module
 *
 * Implements game state, initialization, and collision detection
 *
 **/

#include "game.h"
#include "movement.h"
#include "fruit.h"
#include "screen.h"

// Global game state
struct GameState game;

/**
 *
 * Function: init_random
 *
 * Deliverable:
 * ------------
 * Initialize the random number generator with a seed value
 *
 * Pseudocode:
 *
 * 1. Set initial seed value
 *
 **/
void init_random(void) {
    // 1. Set initial seed value
    game.random_seed = 12345;
}

/**
 *
 * Function: random_int
 *
 * Deliverable:
 * ------------
 * Generate a pseudo-random integer using Linear Congruential Generator
 *
 * Pseudocode:
 *
 * 1. Update seed using LCG formula
 * 2. Return the new seed value
 *
 **/
int random_int(void) {
    // 1. Update seed using LCG formula
    game.random_seed = (game.random_seed * 1103515245 + 12345) & 0x7fffffff;
    
    // 2. Return the new seed value
    return game.random_seed;
}

/**
 *
 * Function: random_range
 *
 * Deliverable:
 * ------------
 * Generate a random integer within a specified range
 *
 * @param min - Minimum value (inclusive)
 * @param max - Maximum value (inclusive)
 *
 * Pseudocode:
 *
 * 1. Generate random integer
 * 2. Map to range using modulo
 * 3. Return result
 *
 **/
int random_range(int min, int max) {
    // 1-2. Generate random integer and map to range
    return (random_int() % (max - min + 1)) + min;
}

/**
 *
 * Function: init_snake
 *
 * Deliverable:
 * ------------
 * Initialize snake position in center of grid with length of 3
 *
 * Pseudocode:
 *
 * 1. Set initial snake length to 3
 * 2. Calculate center position of grid
 * 3. Set head position at center
 * 4. Set remaining segments to the left of head
 * 5. Set initial direction to right
 *
 **/
void init_snake(void) {
    // 1. Set initial snake length to 3
    game.snake_length = 3;
    
    // 2. Calculate center position of grid
    int center_x = GRID_WIDTH / 2;
    int center_y = GRID_HEIGHT / 2;
    
    // 3. Set head position at center
    game.snake_body[0].x = center_x;
    game.snake_body[0].y = center_y;
    
    // 4. Set remaining segments to the left of head
    game.snake_body[1].x = center_x - 1;
    game.snake_body[1].y = center_y;
    game.snake_body[2].x = center_x - 2;
    game.snake_body[2].y = center_y;
    
    // 5. Set initial direction to right
    game.current_direction = DIR_RIGHT;
}

/**
 *
 * Function: check_wall_collision
 *
 * Deliverable:
 * ------------
 * Check if snake head has collided with grid boundaries
 *
 * Pseudocode:
 *
 * 1. Get head x and y coordinates
 * 2. Check if x is outside grid width
 * 3. Check if y is outside grid height
 * 4. Return 1 if collision, 0 otherwise
 *
 **/
int check_wall_collision(void) {
    // 1. Get head coordinates
    int head_x = game.snake_body[0].x;
    int head_y = game.snake_body[0].y;
    
    // 2-3. Check boundaries
    if(head_x < 0 || head_x >= GRID_WIDTH ||
       head_y < 0 || head_y >= GRID_HEIGHT) {
        // 4. Return collision detected
        return 1;
    }
    
    // 4. Return no collision
    return 0;
}

/**
 *
 * Function: check_self_collision
 *
 * Deliverable:
 * ------------
 * Check if snake head has collided with its own body
 *
 * Pseudocode:
 *
 * 1. Get head coordinates
 * 2. Loop through all body segments except head
 * 3. Compare each segment with head position
 * 4. Return 1 if collision found, 0 otherwise
 *
 **/
int check_self_collision(void) {
    // 1. Get head coordinates
    int head_x = game.snake_body[0].x;
    int head_y = game.snake_body[0].y;
    
    // 2-3. Loop through body segments and compare
    for(int i = 1; i < game.snake_length; i++) {
        if(game.snake_body[i].x == head_x && 
           game.snake_body[i].y == head_y) {
            // 4. Return collision detected
            return 1;
        }
    }
    
    // 4. Return no collision
    return 0;
}

/**
 *
 * Function: check_win_condition
 *
 * Deliverable:
 * ------------
 * Check if snake has filled the entire screen
 *
 * Pseudocode:
 *
 * 1. Check if snake length equals grid size
 * 2. Return 1 if win, 0 otherwise
 *
 **/
int check_win_condition(void) {
    // 1-2. Check if snake fills entire grid
    if(game.snake_length >= MAX_SNAKE_LENGTH) {
        return 1;
    }
    return 0;
}

/**
 *
 * Function: read_keyboard_input
 *
 * Deliverable:
 * ------------
 * Read keyboard input from port 0x60 and update game state
 *
 * Pseudocode:
 *
 * 1. Check if keyboard has data ready
 * 2. Read scancode
 * 3. Handle input based on current game state
 *
 **/
void read_keyboard_input(void) {
    // 1. Check if keyboard has data ready
    uint8_t status = inb(0x64);
    if(status & 1) {
        // 2. Read scancode from port 0x60
        uint8_t scancode = inb(0x60);
        
        // 3. Handle input based on game state
        if(game.game_state == STATE_TITLE_SCREEN) {
            if(scancode == 0x1C) {  // Enter key
                game.game_state = STATE_HOW_TO_PLAY;
                draw_how_to_play_screen();
            }
            return;
        }
        
        if(game.game_state == STATE_HOW_TO_PLAY) {
            if(scancode == 0x1C) {  // Enter key
                game.game_state = STATE_PLAYING;
                clear_screen();
                reset_render_flag();  // Reset the first_render flag
                draw_initial_game_state();
            }
            return;
        }
        
        if(game.game_state == STATE_GAME_OVER || game.game_state == STATE_YOU_WIN) {
            if(scancode == 0x1C) {  // Enter key - restart game
                init_game();  // Reset snake position and game state
                game.game_state = STATE_TITLE_SCREEN;
                draw_title_screen();
            }
            return;
        }
        
        if(game.game_state == STATE_PLAYING) {
            handle_direction_input(scancode);
        }
    }
}

/**
 *
 * Function: init_game
 *
 * Deliverable:
 * ------------
 * Initialize all game components
 *
 * Pseudocode:
 *
 * 1. Initialize random number generator
 * 2. Initialize snake
 * 3. Mark fruit as inactive initially
 * 4. Spawn initial fruit
 * 5. Reset score
 *
 **/
void init_game(void) {
    // 1. Initialize random number generator
    init_random();
    
    // 2. Initialize snake
    init_snake();
    
    // 3. Mark fruit as inactive initially
    game.fruit_active = 0;
    
    // 4. Spawn initial fruit
    spawn_fruit();
    
    // 5. Reset score
    game.score = 0;
}

/**
 *
 * Function: game_loop
 *
 * Deliverable:
 * ------------
 * Main game loop that runs continuously through all game states
 *
 * Pseudocode:
 *
 * 1. Initialize game
 * 2. Set initial state to title screen
 * 3. Enter infinite loop
 * 4. Handle different game states
 *
 **/
void game_loop(void) {
    int frame_counter = 0;
    int game_speed = 2000;
    int test_mode = 0;  // Set to 1 to test win condition
    
    // 1. Initialize game
    init_game();
    
    // TEST MODE: Fill snake to test win screen
    if(test_mode) {
        // Grid is 12x9 = 108 cells total
        // Start with 100 cells filled, leaving 8 for gameplay
        game.snake_length = 100;
        
        // Fill from top-left going right, then down
        int idx = 0;
        for(int y = 0; y < GRID_HEIGHT && idx < game.snake_length; y++) {
            for(int x = 0; x < GRID_WIDTH && idx < game.snake_length; x++) {
                game.snake_body[idx].x = x;
                game.snake_body[idx].y = y;
                idx++;
            }
        }
        
        // Set direction to right (safe direction)
        game.current_direction = DIR_RIGHT;
        
        // Place fruit in an empty spot near the end
        // Find first empty cell
        for(int y = 0; y < GRID_HEIGHT; y++) {
            for(int x = 0; x < GRID_WIDTH; x++) {
                int occupied = 0;
                for(int i = 0; i < game.snake_length; i++) {
                    if(game.snake_body[i].x == x && game.snake_body[i].y == y) {
                        occupied = 1;
                        break;
                    }
                }
                if(!occupied) {
                    game.fruit.x = x;
                    game.fruit.y = y;
                    game.fruit_active = 1;
                    goto done_placing_fruit;
                }
            }
        }
        done_placing_fruit:;
    }
    
    // 2. Set initial state to title screen
    game.game_state = STATE_TITLE_SCREEN;
    draw_title_screen();
    
    // 3. Enter infinite loop
    while(1) {
        // Read keyboard input
        read_keyboard_input();
        
        // 4. Handle different game states
        if(game.game_state == STATE_TITLE_SCREEN || 
           game.game_state == STATE_HOW_TO_PLAY ||
           game.game_state == STATE_GAME_OVER ||
           game.game_state == STATE_YOU_WIN) {
            delay(100000);
            continue;
        }
        
        if(game.game_state == STATE_PLAYING) {
            frame_counter++;
            if(frame_counter >= game_speed) {
                move_snake();
                
                // Check fruit BEFORE checking death collisions
                check_fruit_collision();
                
                // Check win condition first
                if(check_win_condition()) {
                    game.game_state = STATE_YOU_WIN;
                    draw_you_win_screen();
                }
                // Check death collisions AFTER fruit check
                else if(check_wall_collision() || check_self_collision()) {
                    game.game_state = STATE_GAME_OVER;
                    draw_game_over_screen();
                } else {
                    render_game();
                }
                
                frame_counter = 0;
            }
            delay(10000);
        }
    }
}
