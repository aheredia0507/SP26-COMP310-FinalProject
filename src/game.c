/**
 *
 * Final Project - Snake Game - Game Module
 *
 * Implements game state, initialization, collision detection,
 * keyboard input handling, and the main game loop
 *
 * Random number generation uses an LCG (Linear Congruential Generator)
 * seeded from the PIT (Programmable Interval Timer) counter at the moment
 * the player presses Enter on the title screen. Because the seed depends
 * on the exact CPU cycle the player acts, it is non-deterministic even
 * though the underlying LCG algorithm is deterministic.
 *
 * Reference: OS book Section 3.3 (polling), Section 3.4 (interrupt-driven I/O)
 *            HW3 interrupt.c - pit_handler and IDT setup
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
 * Initialize the random number generator seed to zero.
 * The seed is set to a meaningful value later by seed_random_from_pit()
 * when the player presses Enter on the title screen.
 * Separating init from seeding makes it clear that true entropy
 * only arrives once the player interacts with the system.
 *
 * Pseudocode:
 *
 * 1. Set random seed to 0 (placeholder until PIT seed is captured)
 *
 **/

void init_random(void) {

    // 1. Zero the seed - will be replaced by PIT value on first keypress
    game.random_seed = 0;
}

/**
 *
 * Function: seed_random_from_pit
 *
 * Deliverable:
 * ------------
 * Seed the LCG random number generator using the current PIT counter value.
 *
 * The PIT (Programmable Interval Timer) channel 0 runs continuously from
 * boot. Reading it at the exact moment the player presses Enter gives us
 * a value that depends on unpredictable human reaction time, making each
 * game session start with a different fruit sequence even though the LCG
 * itself is purely deterministic.
 *
 *
 * Reference: OS book Table 3.1 - PIT registers 0x40 through 0x43
 *
 * Pseudocode:
 *
 * 1. Read current PIT channel 0 counter via read_pit_count()
 * 2. Store the counter value as the new random seed
 *
 **/

void seed_random_from_pit(void) {

    // 1. Read the PIT channel 0 counter (see kernel_main.c: read_pit_count)
    uint16_t pit_value = read_pit_count();

    // 2. Use the PIT counter as the LCG seed
    //    Each player presses Enter at a different CPU cycle, so pit_value
    //    differs every run - this is our entropy source
    game.random_seed = (unsigned int)pit_value;
}

/**
 *
 * Function: random_int
 *
 * Deliverable:
 * ------------
 * Generate a pseudo-random integer using a Linear Congruential Generator.
 *
 * LCG formula: seed = (seed * multiplier + increment) & mask
 *
 * Constants used are the glibc standard LCG parameters:
 *   multiplier = 1103515245
 *   increment  = 12345
 *   mask       = 0x7fffffff  (31-bit output)
 *
 * The LCG is deterministic - given the same seed it always produces the
 * same sequence. The seed from seed_random_from_pit() ensures that each
 * game starts from a different point in that sequence.
 *
 * Pseudocode:
 *
 * 1. Update seed using LCG formula
 * 2. Return the new seed value as the random integer
 *
 **/

int random_int(void) {

    // 1. Update seed using LCG formula (glibc standard constants)
    game.random_seed = (game.random_seed * 1103515245 + 12345) & 0x7fffffff;

    // 2. Return the updated seed as the random result
    return game.random_seed;
}

/**
 *
 * Function: random_range
 *
 * Deliverable:
 * ------------
 * Generate a random integer within a specified inclusive range [min, max].
 *
 * @param min - Minimum value (inclusive)
 * @param max - Maximum value (inclusive)
 *
 * Pseudocode:
 *
 * 1. Generate a random integer via random_int()
 * 2. Map the result into [min, max] using modulo and addition
 * 3. Return the result
 *
 **/

int random_range(int min, int max) {

    // 1-2. Generate random int and map to [min, max]
    return (random_int() % (max - min + 1)) + min;

    // 3. (returned above)
}

/**
 *
 * Function: init_snake
 *
 * Deliverable:
 * ------------
 * Initialize snake position in the center of the grid with a length of 3,
 * facing right.
 *
 * Pseudocode:
 *
 * 1. Set initial snake length to 3
 * 2. Calculate center position of the grid
 * 3. Set head position at center
 * 4. Set remaining segments to the left of the head
 * 5. Set initial direction to right
 *
 **/

void init_snake(void) {

    // 1. Set initial snake length to 3
    game.snake_length = 3;

    // 2. Calculate center position of the grid
    int center_x = GRID_WIDTH / 2;
    int center_y = GRID_HEIGHT / 2;

    // 3. Set head position at center
    game.snake_body[0].x = center_x;
    game.snake_body[0].y = center_y;

    // 4. Set remaining segments to the left of the head
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
 * Check if the snake head has collided with the grid boundaries.
 *
 * Pseudocode:
 *
 * 1. Get head x and y coordinates
 * 2. Check if x is outside grid width
 * 3. Check if y is outside grid height
 * 4. Return 1 if collision detected, 0 otherwise
 *
 **/

int check_wall_collision(void) {

    // 1. Get head coordinates
    int head_x = game.snake_body[0].x;
    int head_y = game.snake_body[0].y;

    // 2-3. Check all boundaries
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
 * Check if the snake head has collided with its own body.
 *
 * Pseudocode:
 *
 * 1. Get head coordinates
 * 2. Loop through all body segments except the head
 * 3. Compare each segment position to the head position
 * 4. Return 1 if a match is found, 0 otherwise
 *
 **/

int check_self_collision(void) {

    // 1. Get head coordinates
    int head_x = game.snake_body[0].x;
    int head_y = game.snake_body[0].y;

    // 2-3. Loop through body segments and compare to head
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
 * Check if the snake has grown to fill the entire grid.
 *
 * Pseudocode:
 *
 * 1. Check if snake length has reached or exceeded the maximum grid size
 * 2. Return 1 if win condition is met, 0 otherwise
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
 * Poll the PS/2 keyboard controller and update game state based on
 * the current scancode. Handles all game state transitions.
 *
 * Keyboard I/O uses the same polling approach described in the OS book
 * (Section 3.3, Exercise 3.1): read the PS/2 status register at port
 * 0x64 to check if data is ready, then read the scancode from port 0x60.
 *
 * When the player presses Enter on the title screen,
 * seed_random_from_pit() is called before transitioning. This captures
 * the PIT counter at the exact moment of the keypress, giving us a
 * non-deterministic seed based on the player's timing.
 *
 * Pseudocode:
 *
 * 1. Read PS/2 status register from port 0x64
 * 2. Check if bit 0 (output buffer full) is set - if not, return early
 * 3. Read scancode from port 0x60
 * 4. Handle scancode based on current game state:
 *    a. Title screen - Enter seeds RNG from PIT then shows how-to-play
 *    b. How-to-play  - Enter transitions to playing state
 *    c. Game over / win - Enter resets and returns to title screen
 *    d. Playing - pass scancode to direction handler
 *
 **/

void read_keyboard_input(void) {

    // 1. Read PS/2 status register (OS book Section 3.3, port 0x64)
    uint8_t status = inb(0x64);

    // 2. Check if output buffer has data (bit 0 of status register)
    if(status & 1) {

        // 3. Read scancode from PS/2 data port (port 0x60)
        uint8_t scancode = inb(0x60);

        // 4a. Title screen: seed RNG from PIT on Enter, then advance
        if(game.game_state == STATE_TITLE_SCREEN) {
            if(scancode == 0x1C) {  // Enter key scancode

                // Capture PIT counter at the exact moment of keypress
                // Human reaction time makes this value unpredictable -
                // this is our entropy source for the random seed
                seed_random_from_pit();

                game.game_state = STATE_HOW_TO_PLAY;
                draw_how_to_play_screen();
            }
            return;
        }

        // 4b. How-to-play screen: Enter transitions to playing state
        if(game.game_state == STATE_HOW_TO_PLAY) {
            if(scancode == 0x1C) {  // Enter key
                game.game_state = STATE_PLAYING;
                clear_screen();
                reset_render_flag();
                draw_initial_game_state();
            }
            return;
        }

        // 4c. Game over or win: Enter resets and returns to title screen
        if(game.game_state == STATE_GAME_OVER ||
           game.game_state == STATE_YOU_WIN) {
            if(scancode == 0x1C) {  // Enter key
                init_game();
                game.game_state = STATE_TITLE_SCREEN;
                draw_title_screen();
            }
            return;
        }

        // 4d. Playing: pass scancode to direction handler
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
 * Initialize all game components to their starting state.
 * Note: init_random() is called here to zero the seed, but the real
 * entropy is captured later in seed_random_from_pit() when the player
 * presses Enter. This way every restart also re-seeds from the PIT.
 *
 * Pseudocode:
 *
 * 1. Initialize random number generator (zero the seed)
 * 2. Initialize snake position and direction
 * 3. Mark fruit as inactive
 * 4. Spawn the initial fruit
 * 5. Reset score to zero
 *
 **/

void init_game(void) {

    // 1. Initialize random number generator (seed zeroed here,
    //    PIT entropy captured on next title screen Enter press)
    init_random();

    // 2. Initialize snake position and direction
    init_snake();

    // 3. Mark fruit as inactive initially
    game.fruit_active = 0;

    // 4. Spawn the initial fruit
    spawn_fruit();

    // 5. Reset score to zero
    game.score = 0;
}

/**
 *
 * Function: setup_test_mode
 *
 * Deliverable:
 * ------------
 * Configure the game state to test the win screen without playing a
 * full game. Fills the grid to one fruit away from MAX_SNAKE_LENGTH,
 * placing the head and fruit so the very next frame triggers a win.
 *
 * Pseudocode:
 *
 * 1. Set snake_length to MAX_SNAKE_LENGTH - 1 (one fruit away from win)
 * 2. Fill body[1] through body[snake_length-1] left-to-right top-to-bottom
 *    leaving the last two cells of the bottom row open
 * 3. Place head (body[0]) at second-to-last cell of the bottom row
 * 4. Set direction to RIGHT so head moves toward fruit on first frame
 * 5. Place fruit at the last cell of the bottom row, one step ahead
 * 6. Mark fruit as active
 *
 **/

void setup_test_mode(void) {

    // 1. Set snake length to one fruit away from the win condition
    game.snake_length = MAX_SNAKE_LENGTH - 1;

    // 2. Fill body segments [1..snake_length-1] left-to-right, top-to-bottom
    int idx = 1;
    for(int y = 0; y < GRID_HEIGHT && idx < game.snake_length; y++) {
        for(int x = 0; x < GRID_WIDTH && idx < game.snake_length; x++) {
            game.snake_body[idx].x = x;
            game.snake_body[idx].y = y;
            idx++;
        }
    }

    // 3. Place head at second-to-last cell of the bottom row (10, 8)
    game.snake_body[0].x = GRID_WIDTH - 2;
    game.snake_body[0].y = GRID_HEIGHT - 1;

    // 4. Set direction RIGHT - head reaches fruit on the next frame
    game.current_direction = DIR_RIGHT;

    // 5. Place fruit at last cell of the bottom row (11, 8)
    game.fruit.x = GRID_WIDTH - 1;
    game.fruit.y = GRID_HEIGHT - 1;

    // 6. Mark fruit as active
    game.fruit_active = 1;
}


/**
 *
 * Function: game_loop
 *
 * Deliverable:
 * ------------
 * Main game loop. Runs continuously, polling for keyboard input and
 * advancing the game state each time the PIT timer fires an interrupt.
 *
 * Snake movement is now driven by the timer_tick flag set by pit_handler
 * in interrupt.c rather than a busy-wait frame counter.
 * The PIT fires at ~18 Hz base rate. pit_handler divides this down
 * by a threshold of 3, giving the snake ~6 moves per second.
 *
 * This replaces the polling approach previously implemented with an
 * interrupt-driven approach, using the same IDT
 * infrastructure built in HW3.
 *
 * Pseudocode:
 *
 * 1. Initialize all game components
 * 2. If test_mode is set, call setup_test_mode() to configure win screen test
 * 3. Set initial state to title screen and draw it
 * 4. Enter infinite loop:
 *    a. Poll keyboard for input
 *    b. If in a non-playing state, continue
 *    c. If playing, check if timer_tick flag is set by pit_handler
 *    d. If timer_tick is set, clear the flag and advance game:
 *       i.  Move the snake one step
 *       ii. Check fruit collision (grow snake if eaten)
 *       iii.Check win condition
 *       iv. Check death collisions (wall or self)
 *       v.  Render the updated game state
 *
 **/

void game_loop(void) {
    int test_mode = 0;  // Set to 1 to test win screen

    // 1. Initialize all game components
    init_game();

    // 2. If test mode is on, override game state with win-screen setup
    if(test_mode) {
        setup_test_mode();
    }

    // 3. Set initial state and draw title screen
    game.game_state = STATE_TITLE_SCREEN;
    draw_title_screen();

    // 4. Enter infinite loop
    while(1) {

        // 4a. Poll keyboard for input (OS book Section 3.3 polling pattern)
        read_keyboard_input();

        // 4b. Non-playing states just wait for keypress
        if(game.game_state != STATE_PLAYING) {
            continue;
        }

        // 4c. Check if pit_handler has set the timer tick flag
        if(timer_tick) {

            // 4d. Clear the flag before processing so we don't skip a tick
            timer_tick = 0;

            // 4d-i. Move the snake one step in current direction
            move_snake();

            // 4d-ii. Check if snake ate a fruit (grow + spawn new)
            check_fruit_collision();

            // 4d-iii. Check win condition first
            if(check_win_condition()) {
                game.game_state = STATE_YOU_WIN;
                draw_you_win_screen();
            }
            // 4d-iv. Check death collisions after fruit check
            else if(check_wall_collision() || check_self_collision()) {
                game.game_state = STATE_GAME_OVER;
                draw_game_over_screen();
            } else {
                // 4d-v. Render updated game state
                render_game();
            }
        }
    }
}
