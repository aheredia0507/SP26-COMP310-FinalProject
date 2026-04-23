/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game - Screen Module
 *
 * Implements all rendering using frame buffer tracking
 *
 **/

#include "screen.h"

// Frame buffer: tracks what's displayed at each grid position
static uint8_t frame_buffer[GRID_HEIGHT][GRID_WIDTH];

// Simple 5x7 bitmap font - 8 rows for descenders
static const uint8_t font_data[][8] = {
    // Space (32)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // A (65)
    {0x70, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00},
    // C (67)
    {0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00},
    // D (68)
    {0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00},
    // E (69)
    {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00},
    // G (71)
    {0x70, 0x88, 0x80, 0xB8, 0x88, 0x88, 0x70, 0x00},
    // H (72)
    {0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00},
    // I (73)
    {0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00},
    // K (75)
    {0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00},
    // M (77)
    {0x88, 0xD8, 0xA8, 0xA8, 0x88, 0x88, 0x88, 0x00},
    // N (78)
    {0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x88, 0x00},
    // O (79)
    {0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00},
    // P (80)
    {0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00},
    // R (82)
    {0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0x00},
    // S (83)
    {0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70, 0x00},
    // T (84)
    {0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
    // U (85)
    {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00},
    // V (86)
    {0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00},
    // W (87)
    {0x88, 0x88, 0x88, 0xA8, 0xA8, 0xD8, 0x88, 0x00},
    // Y (89)
    {0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20, 0x00},
    // a (97)
    {0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00},
    // c (99)
    {0x00, 0x00, 0x70, 0x80, 0x80, 0x88, 0x70, 0x00},
    // d (100)
    {0x08, 0x08, 0x78, 0x88, 0x88, 0x88, 0x78, 0x00},
    // e (101)
    {0x00, 0x00, 0x70, 0x88, 0xF0, 0x80, 0x70, 0x00},
    // f (102)
    {0x30, 0x48, 0x40, 0xE0, 0x40, 0x40, 0x40, 0x00},
    // g (103)
    {0x00, 0x00, 0x78, 0x88, 0x88, 0x78, 0x08, 0x70},
    // h (104)
    {0x80, 0x80, 0xF0, 0x88, 0x88, 0x88, 0x88, 0x00},
    // i (105)
    {0x20, 0x00, 0x60, 0x20, 0x20, 0x20, 0x70, 0x00},
    // k (107)
    {0x80, 0x80, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x00},
    // l (108)
    {0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00},
    // m (109)
    {0x00, 0x00, 0xD0, 0xA8, 0xA8, 0xA8, 0xA8, 0x00},
    // n (110)
    {0x00, 0x00, 0xF0, 0x88, 0x88, 0x88, 0x88, 0x00},
    // o (111)
    {0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00},
    // p (112)
    {0x00, 0x00, 0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80},
    // r (114)
    {0x00, 0x00, 0xF0, 0x88, 0x80, 0x80, 0x80, 0x00},
    // s (115)
    {0x00, 0x00, 0x70, 0x80, 0x70, 0x08, 0xF0, 0x00},
    // t (116)
    {0x20, 0x20, 0xF8, 0x20, 0x20, 0x20, 0x18, 0x00},
    // u (117)
    {0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x78, 0x00},
    // v (118)
    {0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00},
    // w (119)
    {0x00, 0x00, 0x88, 0x88, 0xA8, 0xA8, 0x50, 0x00},
    // y (121)
    {0x00, 0x00, 0x88, 0x88, 0x78, 0x08, 0x88, 0x70},
};

// Simple apple sprite: 30x30 pixel art
// 0 = transparent, 1 = red, 2 = dark red, 3 = brown, 4 = green leaf
static const uint8_t apple_sprite_30[30][30] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,3,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

int get_char_index(char c) {
    if(c == ' ') return 0;
    if(c == 'A') return 1;
    if(c == 'C') return 2;
    if(c == 'D') return 3;
    if(c == 'E') return 4;
    if(c == 'G') return 5;
    if(c == 'H') return 6;
    if(c == 'I') return 7;
    if(c == 'K') return 8;
    if(c == 'M') return 9;
    if(c == 'N') return 10;
    if(c == 'O') return 11;
    if(c == 'P') return 12;
    if(c == 'R') return 13;
    if(c == 'S') return 14;
    if(c == 'T') return 15;
    if(c == 'U') return 16;
    if(c == 'V') return 17;
    if(c == 'W') return 18;
    if(c == 'Y') return 19;
    if(c == 'a') return 20;
    if(c == 'c') return 21;
    if(c == 'd') return 22;
    if(c == 'e') return 23;
    if(c == 'f') return 24;
    if(c == 'g') return 25;
    if(c == 'h') return 26;
    if(c == 'i') return 27;
    if(c == 'k') return 28;
    if(c == 'l') return 29;
    if(c == 'm') return 30;
    if(c == 'n') return 31;
    if(c == 'o') return 32;
    if(c == 'p') return 33;
    if(c == 'r') return 34;
    if(c == 's') return 35;
    if(c == 't') return 36;
    if(c == 'u') return 37;
    if(c == 'v') return 38;
    if(c == 'w') return 39;
    if(c == 'y') return 40;
    return 0;
}

void draw_char(char c, int x, int y, int scale, int color) {
    int idx = get_char_index(c);
    const uint8_t *bitmap = font_data[idx];

    for(int row = 0; row < 8; row++) {
        uint8_t line = bitmap[row];
        for(int col = 0; col < 8; col++) {
            if(line & (0x80 >> col)) {
                for(int sy = 0; sy < scale; sy++) {
                    for(int sx = 0; sx < scale; sx++) {
                        drawPixel(x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

void draw_text(char *text, int x, int y, int scale, int color) {
    int offset = 0;
    while(*text) {
        draw_char(*text, x + offset, y, scale, color);
        offset += 6 * scale;
        text++;
    }
}

void draw_cell(int grid_x, int grid_y, int color) {
    int pixel_x = (grid_x * CELL_SIZE) + OFFSET_X;
    int pixel_y = (grid_y * CELL_SIZE) + OFFSET_Y;

    for(int px = 0; px < CELL_SIZE; px++) {
        for(int py = 0; py < CELL_SIZE; py++) {
            drawPixel(pixel_x + px, pixel_y + py, color);
        }
    }
}

void draw_big_cell(int grid_x, int grid_y, int color) {
    for(int dx = 0; dx < SEGMENT_SIZE; dx++) {
        for(int dy = 0; dy < SEGMENT_SIZE; dy++) {
            draw_cell(grid_x * SEGMENT_SIZE + dx, grid_y * SEGMENT_SIZE + dy, color);
        }
    }
}

void draw_segment(int grid_x, int grid_y, int color) {
    for(int dx = 0; dx < SEGMENT_SIZE; dx++) {
        for(int dy = 0; dy < SEGMENT_SIZE; dy++) {
            draw_cell(grid_x * SEGMENT_SIZE + dx, grid_y * SEGMENT_SIZE + dy, color);
        }
    }
}

int get_background_color(int x, int y) {
    if((x + y) % 2 == 0) {
        return COLOR_BACKGROUND_DARK;
    } else {
        return COLOR_BACKGROUND_LIGHT;
    }
}

/**
 *
 * Function: get_apple_color
 *
 * Deliverable:
 * ------------
 * Map sprite value to RGB color
 *
 * Pseudocode:
 *
 * 1. Switch on sprite value
 * 2. Return corresponding RGB color
 *
 **/
int get_apple_color(uint8_t value) {
    // 1-2. Map values to colors
    switch(value) {
        case 1: return 0xFF0000;  // Red
        case 2: return 0xCC0000;  // Dark red (highlight)
        case 3: return 0x8B4513;  // Brown (stem)
        case 4: return 0x228B22;  // Green (leaf)
        default: return -1;       // Transparent
    }
}

/**
 *
 * Function: draw_apple
 *
 * Deliverable:
 * ------------
 * Draw apple sprite at grid position (scaled to 60x60)
 *
 * @param grid_x - Grid x coordinate
 * @param grid_y - Grid y coordinate
 *
 * Pseudocode:
 *
 * 1. Calculate base pixel position and background color
 * 2. Loop through sprite (30x30)
 * 3. Scale each sprite pixel to 2x2 screen pixels
 * 4. Draw color or background for each pixel
 *
 **/
void draw_apple(int grid_x, int grid_y) {
    // 1. Calculate base position and get grid cell's background color
    int base_x = (grid_x * SEGMENT_SIZE * CELL_SIZE) + OFFSET_X;
    int base_y = (grid_y * SEGMENT_SIZE * CELL_SIZE) + OFFSET_Y;
    
    // Get the background color for this entire grid cell
    int bg_color = get_background_color(grid_x, grid_y);
    
    // 2. Loop through 30x30 sprite
    for(int sy = 0; sy < 30; sy++) {
        for(int sx = 0; sx < 30; sx++) {
            uint8_t pixel = apple_sprite_30[sy][sx];
            int color = get_apple_color(pixel);
            
            // 3. Scale to 2x2 pixels (30x30 sprite -> 60x60 display)
            for(int dy = 0; dy < 2; dy++) {
                for(int dx = 0; dx < 2; dx++) {
                    int px = base_x + sx * 2 + dx;
                    int py = base_y + sy * 2 + dy;
                    
                    // 4. Draw color or background
                    if(color >= 0) {
                        drawPixel(px, py, color);
                    } else {
                        // Transparent - use grid cell's background color
                        drawPixel(px, py, bg_color);
                    }
                }
            }
        }
    }
}

void clear_screen(void) {
    for(int x = 0; x < GRID_WIDTH; x++) {
        for(int y = 0; y < GRID_HEIGHT; y++) {
            draw_big_cell(x, y, get_background_color(x, y));
            frame_buffer[y][x] = CELL_EMPTY;
        }
    }
}

void clear_full_screen(int color) {
    for(int x = 0; x < SCREEN_WIDTH; x++) {
        for(int y = 0; y < SCREEN_HEIGHT; y++) {
            drawPixel(x, y, color);
        }
    }
}

void delay(unsigned int iterations) {
    for(volatile unsigned int i = 0; i < iterations; i++) {
    }
}

void draw_pixel_rect(int x, int y, int width, int height, int color) {
    for(int px = 0; px < width; px++) {
        for(int py = 0; py < height; py++) {
            drawPixel(x + px, y + py, color);
        }
    }
}

void draw_title_screen(void) {
    clear_full_screen(COLOR_BLACK);
    
    int title_x = (SCREEN_WIDTH - 432) / 2;
    int title_y = (SCREEN_HEIGHT - 200) / 2;
    draw_text("SNAKE ARCADE", title_x, title_y, 6, COLOR_SNAKE_HEAD);
    
    int inst_x = (SCREEN_WIDTH - 378) / 2;
    int inst_y = title_y + 150;
    draw_text("Press Enter to Start", inst_x, inst_y, 3, COLOR_TEXT);
}

void draw_how_to_play_screen(void) {
    clear_full_screen(COLOR_BLACK);
    
    int title_x = (SCREEN_WIDTH - 330) / 2;
    draw_text("How to Play", title_x, 150, 5, COLOR_YELLOW);
    
    draw_text("Use arrow keys to move", (SCREEN_WIDTH - 396) / 2, 300, 3, COLOR_TEXT);
    draw_text("Eat red fruit to grow", (SCREEN_WIDTH - 378) / 2, 360, 3, COLOR_TEXT);
    draw_text("Avoid walls and yourself", (SCREEN_WIDTH - 432) / 2, 420, 3, COLOR_TEXT);
    
    draw_text("Press Enter to Continue", (SCREEN_WIDTH - 414) / 2, 550, 3, COLOR_SNAKE_HEAD);
}

void draw_game_over_screen(void) {
    clear_full_screen(COLOR_BLACK);
    
    int gameover_x = (SCREEN_WIDTH - 378) / 2;
    draw_text("GAME OVER", gameover_x, 300, 7, COLOR_FRUIT);
    
    draw_text("Press Enter to Restart", (SCREEN_WIDTH - 414) / 2, 500, 3, COLOR_TEXT);
}

void draw_you_win_screen(void) {
    clear_full_screen(COLOR_BLACK);
    
    int win_x = (SCREEN_WIDTH - 294) / 2;
    draw_text("YOU WIN", win_x, 300, 7, COLOR_YELLOW);
    
    draw_text("Press Enter to Restart", (SCREEN_WIDTH - 414) / 2, 500, 3, COLOR_TEXT);
}

void draw_initial_game_state(void) {
    // Clear frame buffer
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            frame_buffer[y][x] = CELL_EMPTY;
        }
    }
    
    // Mark snake in frame buffer
    for(int i = 0; i < game.snake_length; i++) {
        int x = game.snake_body[i].x;
        int y = game.snake_body[i].y;
        if(i == 0) {
            frame_buffer[y][x] = CELL_SNAKE_HEAD;
        } else {
            frame_buffer[y][x] = CELL_SNAKE_BODY;
        }
    }
    
    // Mark fruit in frame buffer
    if(game.fruit_active) {
        frame_buffer[game.fruit.y][game.fruit.x] = CELL_FRUIT;
    }
    
    // Draw everything based on frame buffer
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            if(frame_buffer[y][x] == CELL_SNAKE_HEAD) {
                draw_segment(x, y, COLOR_SNAKE_HEAD);
            } else if(frame_buffer[y][x] == CELL_SNAKE_BODY) {
                draw_segment(x, y, COLOR_SNAKE_BODY);
            } else if(frame_buffer[y][x] == CELL_FRUIT) {
                draw_apple(x, y);
            } else {
                draw_big_cell(x, y, get_background_color(x, y));
            }
        }
    }
}

/**
 *
 * Function: render_game
 *
 * Deliverable:
 * ------------
 * Render game using frame buffer - only update changed cells
 *
 * Pseudocode:
 *
 * 1. Create new frame buffer state
 * 2. Compare with old frame buffer
 * 3. Only redraw cells that changed
 * 4. Update frame buffer
 *
 **/
void render_game(void) {
    // 1. Create new frame buffer state
    uint8_t new_buffer[GRID_HEIGHT][GRID_WIDTH];
    
    // Initialize new buffer as empty
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            new_buffer[y][x] = CELL_EMPTY;
        }
    }
    
    // Mark snake positions
    for(int i = 0; i < game.snake_length; i++) {
        int x = game.snake_body[i].x;
        int y = game.snake_body[i].y;
        if(i == 0) {
            new_buffer[y][x] = CELL_SNAKE_HEAD;
        } else {
            new_buffer[y][x] = CELL_SNAKE_BODY;
        }
    }
    
    // Mark fruit position
    if(game.fruit_active) {
        new_buffer[game.fruit.y][game.fruit.x] = CELL_FRUIT;
    }
    
    // 2-3. Compare and redraw only changed cells
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            if(new_buffer[y][x] != frame_buffer[y][x]) {
                // Cell changed - redraw it
                if(new_buffer[y][x] == CELL_SNAKE_HEAD) {
                    draw_segment(x, y, COLOR_SNAKE_HEAD);
                } else if(new_buffer[y][x] == CELL_SNAKE_BODY) {
                    draw_segment(x, y, COLOR_SNAKE_BODY);
                } else if(new_buffer[y][x] == CELL_FRUIT) {
                    draw_apple(x, y);
                } else {
                    // Cell is now empty - draw background
                    draw_big_cell(x, y, get_background_color(x, y));
                }
                
                // 4. Update frame buffer
                frame_buffer[y][x] = new_buffer[y][x];
            }
        }
    }
}

void reset_render_flag(void) {
    // Clear frame buffer when resetting
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            frame_buffer[y][x] = CELL_EMPTY;
        }
    }
}
