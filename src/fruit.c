/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game - Fruit Module
 *
 * Implements fruit spawning and collision detection
 *
 **/

#include "fruit.h"
#include "game.h"

/**
 *
 * Function: spawn_fruit
 *
 * Deliverable:
 * ------------
 * Spawn fruit at a random position not occupied by snake
 *
 * Pseudocode:
 *
 * 1. Generate random coordinates
 * 2. Check if position is occupied by snake
 * 3. Repeat until valid position found
 * 4. Mark fruit as active
 *
 **/
void spawn_fruit(void) {
    int valid_position = 0;
    
    while(!valid_position) {
        // 1. Generate random coordinates
        game.fruit.x = random_range(0, GRID_WIDTH - 1);
        game.fruit.y = random_range(0, GRID_HEIGHT - 1);
        
        // 2. Check if position is occupied by snake
        valid_position = 1;
        for(int i = 0; i < game.snake_length; i++) {
            if(game.snake_body[i].x == game.fruit.x && 
               game.snake_body[i].y == game.fruit.y) {
                valid_position = 0;
                break;
            }
        }
    }
    
    // 4. Mark fruit as active
    game.fruit_active = 1;
}

/**
 *
 * Function: check_fruit_collision
 *
 * Deliverable:
 * ------------
 * Check if snake head is on fruit, grow snake and spawn new fruit
 * FIXED: Position new tail segment immediately when growing
 *
 * Pseudocode:
 *
 * 1. Check if fruit is active
 * 2. Check if snake head matches fruit position
 * 3. If match, position new tail segment at current tail
 * 4. Increase snake length
 * 5. Update score
 * 6. Spawn new fruit
 *
 **/
void check_fruit_collision(void) {
    // 1. Only check if fruit is active
    if(!game.fruit_active) return;
    
    // 2. Check if snake head matches fruit position
    if(game.snake_body[0].x == game.fruit.x && 
       game.snake_body[0].y == game.fruit.y) {
        
        // 3. Position new tail segment at current tail location
        // (prevents uninitialized position at (0,0))
        int old_length = game.snake_length;
        game.snake_body[old_length].x = game.snake_body[old_length - 1].x;
        game.snake_body[old_length].y = game.snake_body[old_length - 1].y;
        
        // 4. Increase snake length
        game.snake_length++;
        
        // 5. Update score
        game.score += 10;
        
        // 6. Spawn new fruit
        spawn_fruit();
    }
}
