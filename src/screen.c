/**
 *
 * Final Project - Snake Game - Screen Module
 *
 * Implements all rendering for the snake game using a frame buffer
 * tracking system. All drawing  calls drawPixel() from
 * kernel_main.c which performs memory-mapped I/O to the framebuffer
 * (OS book Section 3.2.1)
 *
 * A custom bitmap font and integer renderer are implemented from scratch
 * since no stdlib is available in a freestanding kernel environment
 * (OS book Section 2.3.3)
 *
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
    // 0
    {0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00},
    // 1
    {0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00},
    // 2
    {0x70, 0x88, 0x08, 0x30, 0x40, 0x80, 0xF8, 0x00},
    // 3
    {0x70, 0x88, 0x08, 0x30, 0x08, 0x88, 0x70, 0x00},
    // 4
    {0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0x00},
    // 5
    {0xF8, 0x80, 0x80, 0xF0, 0x08, 0x88, 0x70, 0x00},
    // 6
    {0x70, 0x88, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00},
    // 7
    {0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0x00},
    // 8
    {0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00},
    // 9
    {0x70, 0x88, 0x88, 0x78, 0x08, 0x88, 0x70, 0x00},
};

// Simple fruit sprite: 30x30 pixel art
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

/**
 *
 * Function: get_char_index
 *
 * Deliverable:
 * ------------
 * Map an ASCII character to its index in the font_data array.
 * Only characters used in the game's text strings are defined.
 * Any character not in the table returns 0 (space) so unknown
 * characters are silently skipped rather than crashing.
 *
 * Since there is no stdlib in a freestanding kernel environment
 * there is no string lookup table available - the mapping is
 * implemented manually as a chain of if statements.
 *
 * @param c - ASCII character to look up
 *
 * Pseudocode:
 *
 * 1. Compare c against each supported character
 * 2. Return the corresponding font_data index if found
 * 3. Return 0 (space) if character is not supported
 *
 **/

int get_char_index(char c) {
     // 1-2. Compare c against each supported character and return its index
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
    if(c == '0') return 41;
    if(c == '1') return 42;
    if(c == '2') return 43;
    if(c == '3') return 44;
    if(c == '4') return 45;
    if(c == '5') return 46;
    if(c == '6') return 47;
    if(c == '7') return 48;
    if(c == '8') return 49;
    if(c == '9') return 50;
    // 3. Character not supported - return 0 (space) to skip silently
    return 0;
}

/**
 *
 * Function: draw_char
 *
 * Deliverable:
 * ------------
 * Draw a single character from the bitmap font at a given pixel
 * coordinate. Each character is stored as 8 rows of 8 bits in
 * font_data. A set bit means draw a pixel, a clear bit means skip.
 *
 * Since no text rendering library exists in a freestanding kernel
 * environment the font and renderer are implemented entirely from
 * scratch. This is a direct consequence of the -ffreestanding compiler
 * flag 
 * (OS book Section 2.3.3.)
 *
 * @param c     - Character to draw
 * @param x     - Pixel x coordinate of top-left corner
 * @param y     - Pixel y coordinate of top-left corner
 * @param scale - Size multiplier (scale 3 = each font pixel is 3x3)
 * @param color - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Look up the character's bitmap row data in font_data
 * 2. Loop through all 8 rows of the bitmap
 * 3. For each row check each of the 8 bit positions
 * 4. If the bit is set, draw a scale x scale block of pixels
 *
 **/

void draw_char(char c, int x, int y, int scale, int color) {

    // 1. Look up bitmap data for this character
    int idx = get_char_index(c);
    const uint8_t *bitmap = font_data[idx];

    // 2. Loop through all 8 rows of the bitmap
    for(int row = 0; row < 8; row++) {
        uint8_t line = bitmap[row];

        // 3. Check each of the 8 bit positions in this row
        for(int col = 0; col < 8; col++) {
            if(line & (0x80 >> col)) {

                // 4. Bit is set - draw a scale x scale block of pixels
                for(int sy = 0; sy < scale; sy++) {
                    for(int sx = 0; sx < scale; sx++) {
                        drawPixel(x + col * scale + sx,
                                  y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

/**
 *
 * Function: draw_int
 *
 * Deliverable:
 * ------------
 * Draw a non-negative integer to the screen by extracting each digit
 * and drawing it using draw_char. Since no stdlib is available there
 * is no printf or itoa - digits are extracted manually using modulo
 * and division.
 *
 * @param n     - Non-negative integer to draw
 * @param x     - Pixel x coordinate of leftmost digit
 * @param y     - Pixel y coordinate
 * @param scale - Font scale factor
 * @param color - RGB color value
 *
 * Pseudocode:
 *
 * 1. Handle zero as a special case - draw '0' and return
 * 2. Extract digits right to left using modulo and store in array
 * 3. Draw digits left to right advancing x by 6 * scale each character
 *
 **/

void draw_int(int n, int x, int y, int scale, int color) {

    // 1. Handle zero explicitly since the loop below would produce nothing
    if(n == 0) {
        draw_char('0', x, y, scale, color);
        return;
    }

    // 2. Extract digits right to left into array
    char digits[6];
    int count = 0;
    while(n > 0) {
        digits[count++] = '0' + (n % 10);
        n /= 10;
    }

    // 3. Draw digits left to right
    for(int i = count - 1; i >= 0; i--) {
        draw_char(digits[i], x, y, scale, color);
        x += 6 * scale;
    }
}

/**
 *
 * Function: draw_text
 *
 * Deliverable:
 * ------------
 * Draw a null-terminated string to the screen by calling draw_char
 * for each character and advancing the x position between characters.
 * Each character occupies 6 * scale pixels of horizontal space.
 *
 * Since no stdlib is available in a freestanding kernel environment
 * there is no printf or puts - string rendering is implemented
 * manually character by character.
 *
 * @param text  - Null-terminated string to draw
 * @param x     - Pixel x coordinate of first character
 * @param y     - Pixel y coordinate
 * @param scale - Font scale factor passed to draw_char
 * @param color - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Start at x offset 0
 * 2. Loop through each character until null terminator
 * 3. Draw current character at current x offset
 * 4. Advance x offset by 6 * scale for next character
 *
 **/

void draw_text(char *text, int x, int y, int scale, int color) {

    // 1. Start at x offset 0
    int offset = 0;

    // 2. Loop through each character until null terminator
    while(*text) {

        // 3. Draw current character at current x position
        draw_char(*text, x + offset, y, scale, color);

        // 4. Advance x offset by 6 * scale for next character
        offset += 6 * scale;
        text++;
    }
}

/**
 *
 * Function: draw_cell
 *
 * Deliverable:
 * ------------
 * Fill a single 30x30 pixel tile at a given grid coordinate with
 * a solid color. Converts grid coordinates to pixel coordinates
 * using CELL_SIZE and OFFSET values defined in snake.h.
 *
 * All drawing ultimately calls drawPixel() which performs
 * memory-mapped I/O to the framebuffer 
 * (OS book Section 3.2.1).
 *
 * @param grid_x - Grid x coordinate
 * @param grid_y - Grid y coordinate
 * @param color  - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Convert grid coordinates to pixel coordinates
 * 2. Loop through all pixels in the 30x30 tile
 * 3. Draw each pixel with the given color
 *
 **/

void draw_cell(int grid_x, int grid_y, int color) {

    // 1. Convert grid coordinates to pixel coordinates
    int pixel_x = (grid_x * CELL_SIZE) + OFFSET_X;
    int pixel_y = (grid_y * CELL_SIZE) + OFFSET_Y;

    // 2-3. Loop through all pixels in the tile and draw each one
    for(int px = 0; px < CELL_SIZE; px++) {
        for(int py = 0; py < CELL_SIZE; py++) {
            drawPixel(pixel_x + px, pixel_y + py, color);
        }
    }
}

/**
 *
 * Function: draw_big_cell
 *
 * Deliverable:
 * ------------
 * Fill one full game grid cell with a solid color by drawing a
 * SEGMENT_SIZE x SEGMENT_SIZE block of 30x30 tiles. Since each
 * game cell is made up of 2x2 tiles the result is a 60x60 pixel
 * filled square on screen.
 *
 * Used to draw background cells and clear grid positions when
 * a snake segment or fruit moves away from a cell.
 *
 * @param grid_x - Game grid x coordinate
 * @param grid_y - Game grid y coordinate
 * @param color  - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Loop through SEGMENT_SIZE columns of tiles
 * 2. Loop through SEGMENT_SIZE rows of tiles
 * 3. Draw each tile using draw_cell at the correct tile coordinate
 *
 **/

void draw_big_cell(int grid_x, int grid_y, int color) {

    // 1-2. Loop through the SEGMENT_SIZE x SEGMENT_SIZE block of tiles
    for(int dx = 0; dx < SEGMENT_SIZE; dx++) {
        for(int dy = 0; dy < SEGMENT_SIZE; dy++) {

            // 3. Draw each tile at its tile-level coordinate
            draw_cell(grid_x * SEGMENT_SIZE + dx,
                      grid_y * SEGMENT_SIZE + dy, color);
        }
    }
}

/**
 *
 * Function: draw_segment
 *
 * Deliverable:
 * ------------
 * Draw one snake segment at a given game grid coordinate by filling
 * a SEGMENT_SIZE x SEGMENT_SIZE block of tiles with a solid color.
 * Functionally identical to draw_big_cell but semantically distinct
 * - draw_big_cell is used for background, draw_segment is used for
 * snake head and body rendering.
 *
 * Color is determined by the caller - COLOR_SNAKE_HEAD for the head
 * and COLOR_SNAKE_BODY for all other segments.
 *
 * @param grid_x - Game grid x coordinate
 * @param grid_y - Game grid y coordinate
 * @param color  - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Loop through SEGMENT_SIZE columns of tiles
 * 2. Loop through SEGMENT_SIZE rows of tiles
 * 3. Draw each tile using draw_cell at the correct tile coordinate
 *
 **/

void draw_segment(int grid_x, int grid_y, int color) {

    // 1-2. Loop through the SEGMENT_SIZE x SEGMENT_SIZE block of tiles
    for(int dx = 0; dx < SEGMENT_SIZE; dx++) {
        for(int dy = 0; dy < SEGMENT_SIZE; dy++) {

            // 3. Draw each tile at its tile-level coordinate
            draw_cell(grid_x * SEGMENT_SIZE + dx,
                      grid_y * SEGMENT_SIZE + dy, color);
        }
    }
}

/**
 *
 * Function: get_background_color
 *
 * Deliverable:
 * ------------
 * Return the background color for a given game grid cell based on
 * a checkerboard pattern. Alternating dark and light colors are
 * determined by whether the sum of the grid coordinates is even
 * or odd, creating a subtle grid texture behind the game.
 *
 * @param x - Game grid x coordinate
 * @param y - Game grid y coordinate
 *
 * Pseudocode:
 *
 * 1. Check if sum of x and y is even or odd
 * 2. Return dark background color if even
 * 3. Return light background color if odd
 *
 **/

int get_background_color(int x, int y) {

    // 1-2. Even sum produces dark background color
    if((x + y) % 2 == 0) {
        return COLOR_BACKGROUND_DARK;
    }

    // 3. Odd sum produces light background color
    return COLOR_BACKGROUND_LIGHT;
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

/**
 *
 * Function: draw_apple_at_pixel
 *
 * Deliverable:
 * ------------
 * Draw the apple sprite at an arbitrary pixel coordinate instead of
 * a grid coordinate. Used by draw_game_over_screen to display the
 * fruit sprite on the menu screen where grid coordinates do not apply.
 *
 * Identical rendering logic to draw_apple but takes raw pixel x/y
 * instead of grid x/y, so the sprite can be placed anywhere on screen.
 *
 * @param pixel_x - Top-left pixel x coordinate of the sprite
 * @param pixel_y - Top-left pixel y coordinate of the sprite
 *
 * Pseudocode:
 *
 * 1. Loop through all 30 rows of the apple sprite
 * 2. Loop through all 30 columns of the apple sprite
 * 3. Get the color value for the current sprite pixel
 * 4. If transparent, draw background color (black for menu screen)
 * 5. If not transparent, draw the apple color
 * 6. Each sprite pixel is scaled to 2x2 screen pixels
 *
 **/
void draw_apple_at_pixel(int pixel_x, int pixel_y) {

    // 1-2. Loop through all sprite pixels
    for(int row = 0; row < 30; row++) {
        for(int col = 0; col < 30; col++) {

            // 3. Get color value for this sprite pixel
            int color = get_apple_color(apple_sprite_30[row][col]);

            // 4-5. Draw 2x2 block for each sprite pixel
            for(int sy = 0; sy < 2; sy++) {
                for(int sx = 0; sx < 2; sx++) {

                    // 4. Transparent pixels use black background
                    if(color == -1) {
                        drawPixel(pixel_x + col * 2 + sx,
                                  pixel_y + row * 2 + sy,
                                  COLOR_BLACK);
                    } else {
                        // 5. Draw apple color
                        drawPixel(pixel_x + col * 2 + sx,
                                  pixel_y + row * 2 + sy,
                                  color);
                    }
                }
            }
        }
    }
}

/**
 *
 * Function: clear_screen
 *
 * Deliverable:
 * ------------
 * Clear the game area by redrawing every grid cell with its
 * checkerboard background color and resetting the frame buffer
 * to CELL_EMPTY. Called when transitioning from a menu screen
 * into active gameplay so the render system starts from a clean
 * known state.
 *
 * Only clears the game grid area defined by GRID_WIDTH and
 * GRID_HEIGHT - does not touch the full 1024x768 screen.
 *
 * Pseudocode:
 *
 * 1. Loop through every cell in the game grid
 * 2. Draw the checkerboard background color for each cell
 * 3. Reset the frame buffer entry for each cell to CELL_EMPTY
 *
 **/

void clear_screen(void) {

    // 1. Loop through every cell in the game grid
    for(int x = 0; x < GRID_WIDTH; x++) {
        for(int y = 0; y < GRID_HEIGHT; y++) {

            // 2. Draw checkerboard background color for this cell
            draw_big_cell(x, y, get_background_color(x, y));

            // 3. Reset frame buffer entry to empty
            frame_buffer[y][x] = CELL_EMPTY;
        }
    }
}

/**
 *
 * Function: clear_full_screen
 *
 * Deliverable:
 * ------------
 * Fill every pixel of the entire 1024x768 screen with a single
 * color. Used by menu screens which take up the full display
 * rather than just the game grid area.
 *
 * Directly calls drawPixel() for every screen pixel which is
 * memory-mapped I/O to the framebuffer
 * (OS book Section 3.2.1.)
 *
 * @param color - 32-bit RGB color value to fill the screen with
 *
 * Pseudocode:
 *
 * 1. Loop through every x coordinate on the screen
 * 2. Loop through every y coordinate on the screen
 * 3. Draw each pixel with the given color
 *
 **/

void clear_full_screen(int color) {

    // 1. Loop through every x coordinate on the screen
    for(int x = 0; x < SCREEN_WIDTH; x++) {

        // 2. Loop through every y coordinate on the screen
        for(int y = 0; y < SCREEN_HEIGHT; y++) {

            // 3. Draw each pixel with the given color
            drawPixel(x, y, color);
        }
    }
}

/**
 *
 * Function: delay
 *
 * Deliverable:
 * ------------
 * Busy-wait loop that burns a given number of CPU cycles doing
 * nothing. Used on menu screens to prevent the input polling loop
 * from running faster than necessary.
 *
 * Polling I/O pattern where the CPU 
 * continuously checks a condition rather than waiting
 * for a hardware interrupt. Snake movement no longer uses this
 * function - it is driven by the PIT interrupt in interrupt.c.
 * The volatile keyword prevents the compiler from optimizing the
 * loop away since it has no observable side effects.
 * (OS book Section 3.3)
 *
 * @param iterations - Number of empty loop iterations to burn
 *
 * Pseudocode:
 *
 * 1. Loop for the given number of iterations doing nothing
 *
 **/

void delay(unsigned int iterations) {

    // 1. Burn iterations - volatile prevents compiler from removing loop
    for(volatile unsigned int i = 0; i < iterations; i++) {
    }
}

/**
 *
 * Function: draw_pixel_rect
 *
 * Deliverable:
 * ------------
 * Fill a rectangle of pixels at a given pixel coordinate with a
 * solid color. Takes raw pixel coordinates and dimensions rather
 * than grid coordinates, allowing rectangles to be drawn anywhere
 * on the screen regardless of the game grid.
 *
 * @param x      - Pixel x coordinate of top-left corner
 * @param y      - Pixel y coordinate of top-left corner
 * @param width  - Width of rectangle in pixels
 * @param height - Height of rectangle in pixels
 * @param color  - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Loop through all columns in the rectangle
 * 2. Loop through all rows in the rectangle
 * 3. Draw each pixel with the given color
 *
 **/

void draw_pixel_rect(int x, int y, int width, int height, int color) {

    // 1. Loop through all columns in the rectangle
    for(int px = 0; px < width; px++) {

        // 2. Loop through all rows in the rectangle
        for(int py = 0; py < height; py++) {

            // 3. Draw each pixel with the given color
            drawPixel(x + px, y + py, color);
        }
    }
}

/**
 *
 * Function: draw_title_screen
 *
 * Deliverable:
 * ------------
 * Draw the title screen showing the game name and a prompt to
 * press Enter to start. Text is centered horizontally by calculating
 * the pixel width of each string and subtracting from SCREEN_WIDTH.
 * Called once when the game first starts and again after a restart.
 * (OS book Section 3.2.1)
 *
 * Pseudocode:
 *
 * 1. Clear the full screen to black
 * 2. Calculate centered x position for title text
 * 3. Draw title text centered in the upper half of the screen
 * 4. Calculate centered x position for instruction text
 * 5. Draw instruction text below the title
 *
 **/

void draw_title_screen(void) {

    // 1. Clear the full screen to black
    clear_full_screen(COLOR_BLACK);

    // 2. Calculate centered x position for title text
    //    "SNAKE ARCADE" = 12 chars x 6 pixels x scale 6 = 432px wide
    int title_x = (SCREEN_WIDTH - 432) / 2;
    int title_y = (SCREEN_HEIGHT - 200) / 2;

    // 3. Draw title text centered in the upper half of the screen
    draw_text("SNAKE ARCADE", title_x, title_y, 6, COLOR_SNAKE_HEAD);

    // 4. Calculate centered x position for instruction text
    //    "Press Enter to Start" = 20 chars x 6 pixels x scale 3 = 360px wide
    int inst_x = (SCREEN_WIDTH - 360) / 2;
    int inst_y = title_y + 150;

    // 5. Draw instruction text below the title
    draw_text("Press Enter to Start", inst_x, inst_y, 3, COLOR_TEXT);
}

/**
 *
 * Function: draw_how_to_play_screen
 *
 * Deliverable:
 * ------------
 * Draw the how to play screen showing the game title, instructions,
 * and a prompt to press Enter to continue. Each instruction line is
 * centered horizontally by calculating its pixel width and subtracting
 * from SCREEN_WIDTH. Called when the player presses Enter on the
 * title screen.
 * (OS book Section 3.2.1)
 *
 * Pseudocode:
 *
 * 1. Clear the full screen to black
 * 2. Draw centered title text at the top of the screen
 * 3. Draw each instruction line centered below the title
 * 4. Draw centered continue prompt at the bottom of the screen
 *
 **/

void draw_how_to_play_screen(void) {

    // 1. Clear the full screen to black
    clear_full_screen(COLOR_BLACK);

    // 2. Draw centered title text at the top of the screen
    //    "How to Play" = 11 chars x 6 pixels x scale 5 = 330px wide
    int title_x = (SCREEN_WIDTH - 330) / 2;
    draw_text("How to Play", title_x, 150, 5, COLOR_YELLOW);

    // 3. Draw each instruction line centered below the title
    //    "Use arrow keys to move" = 22 chars x 6 pixels x scale 3 = 396px wide
    draw_text("Use arrow keys to move", (SCREEN_WIDTH - 396) / 2, 300, 3, COLOR_TEXT);

    //    "Eat red fruit to grow" = 21 chars x 6 pixels x scale 3 = 378px wide
    draw_text("Eat red fruit to grow", (SCREEN_WIDTH - 378) / 2, 360, 3, COLOR_TEXT);

    //    "Avoid walls and yourself" = 24 chars x 6 pixels x scale 3 = 432px wide
    draw_text("Avoid walls and yourself", (SCREEN_WIDTH - 432) / 2, 420, 3, COLOR_TEXT);

    // 4. Draw centered continue prompt at the bottom of the screen
    //    "Press Enter to Continue" = 23 chars x 6 pixels x scale 3 = 414px wide
    draw_text("Press Enter to Continue", (SCREEN_WIDTH - 414) / 2, 550, 3, COLOR_SNAKE_HEAD);
}

/**
 *
 * Function: draw_game_over_screen
 *
 * Deliverable:
 * ------------
 * Draw the game over screen showing the GAME OVER title, the number
 * of fruits eaten displayed next to the apple sprite, and a restart
 * prompt. The apple sprite and score are centered together as a group
 * beneath the title.
 *
 * Score display uses draw_apple_at_pixel and draw_int since no stdlib
 * is available in a freestanding kernel environment - printf does not
 * exist so integer rendering must be implemented manually.
 * (OS book Section 2.3.3)
 *
 * Pseudocode:
 *
 * 1. Clear screen to black
 * 2. Draw "GAME OVER" centered at upper portion of screen
 * 3. Calculate center position for apple sprite and score as a group
 *    a. Apple sprite is 60px wide (30 sprite pixels x 2 scale)
 *    b. Gap between sprite and number is 20px
 *    c. Score digits width depends on value (1-3 digits at scale 4)
 *    d. Total group width = 60 + 20 + digit_width
 * 4. Draw apple sprite at left side of group
 * 5. Draw fruit count number to the right of the sprite
 * 6. Draw "Press Enter to Restart" prompt at bottom
 *
 **/

void draw_game_over_screen(void) {

    // 1. Clear screen to black
    clear_full_screen(COLOR_BLACK);

    // 2. Draw "GAME OVER" centered at y=220
    int gameover_x = (SCREEN_WIDTH - 378) / 2;
    draw_text("GAME OVER", gameover_x, 220, 7, COLOR_FRUIT);

    // 3. Calculate group width for centering apple + score together
    //    Apple sprite renders at 60x60px (30 sprite cols x 2 scale)
    //    Score digit width = number of digits x 6 pixels x scale 4
    int sprite_width = 60;
    int gap = 20;
    int digit_scale = 4;
    int digit_count = 0;
    int temp = game.score;

    // 3a. Count digits in score (minimum 1 for score of 0)
    if(temp == 0) {
        digit_count = 1;
    } else {
        while(temp > 0) {
            digit_count++;
            temp /= 10;
        }
    }

    // 3b. Calculate total group width and starting x to center it
    int score_width = digit_count * 6 * digit_scale;
    int group_width = sprite_width + gap + score_width;
    int group_x = (SCREEN_WIDTH - group_width) / 2;
    int group_y = 370;

    // 4. Draw apple sprite at left side of group
    draw_apple_at_pixel(group_x, group_y);

    // 5. Draw fruit count to the right of the sprite, vertically centered
    //    Sprite is 60px tall, digit height at scale 4 is 8*4=32px
    //    Vertical offset = (60 - 32) / 2 = 14px to center digits on sprite
    draw_int(game.score, group_x + sprite_width + gap,
             group_y + 14, digit_scale, COLOR_TEXT);

    // 6. Draw restart prompt at bottom
    draw_text("Press Enter to Restart", (SCREEN_WIDTH - 414) / 2, 520, 3, COLOR_TEXT);
}

/**
 *
 * Function: draw_you_win_screen
 *
 * Deliverable:
 * ------------
 * Draw the win screen showing the YOU WIN title, the number of fruits
 * eaten displayed next to the apple sprite, and a restart prompt.
 * Displayed when the snake fills the entire grid and
 * check_win_condition() returns 1 in game_loop(). Since winning means
 * eating the maximum possible fruits, the score displayed here will
 * always be 105.
 * (OS book Section 3.2.1)
 *
 * Pseudocode:
 *
 * 1. Clear the full screen to black
 * 2. Calculate centered x position for win text
 * 3. Draw win text centered on the screen
 * 4. Draw centered restart prompt below the win text
 *
 **/

void draw_you_win_screen(void) {

    // 1. Clear the full screen to black
    clear_full_screen(COLOR_BLACK);

    // 2. Draw "YOU WIN" centered at y=220
    int win_x = (SCREEN_WIDTH - 294) / 2;
    draw_text("YOU WIN", win_x, 220, 7, COLOR_YELLOW);

    // 3. Calculate group width for centering apple + score together
    int sprite_width = 60;
    int gap = 20;
    int digit_scale = 4;
    int digit_count = 0;
    int temp = game.score;

    // 3a. Count digits in score (minimum 1 for score of 0)
    if(temp == 0) {
        digit_count = 1;
    } else {
        while(temp > 0) {
            digit_count++;
            temp /= 10;
        }
    }

    // 3b. Calculate total group width and starting x to center it
    int score_width = digit_count * 6 * digit_scale;
    int group_width = sprite_width + gap + score_width;
    int group_x = (SCREEN_WIDTH - group_width) / 2;
    int group_y = 370;

    // 4. Draw apple sprite at left side of group
    draw_apple_at_pixel(group_x, group_y);

    // 5. Draw fruit count to the right of the sprite, vertically centered
    draw_int(game.score, group_x + sprite_width + gap,
             group_y + 14, digit_scale, COLOR_TEXT);

    // 6. Draw restart prompt at bottom
    draw_text("Press Enter to Restart", (SCREEN_WIDTH - 414) / 2, 520, 3, COLOR_TEXT);
}

/**
 *
 * Function: draw_initial_game_state
 *
 * Deliverable:
 * ------------
 * Perform a full redraw of the entire game grid at the start of
 * a game session. Builds the frame buffer from scratch and renders
 * every cell based on current game state. Called once when the
 * player presses Enter on the how to play screen to transition
 * into active gameplay.
 *
 * Unlike render_game which only redraws changed cells, this function
 * redraws every cell unconditionally to ensure a clean starting state.
 * (OS book Section 3.2.1)
 *
 * Pseudocode:
 *
 * 1. Reset all frame buffer entries to CELL_EMPTY
 * 2. Mark snake body positions in the frame buffer
 * 3. Mark fruit position in the frame buffer if active
 * 4. Draw every cell based on its frame buffer entry
 *
 **/

void draw_initial_game_state(void) {

    // 1. Reset all frame buffer entries to CELL_EMPTY
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            frame_buffer[y][x] = CELL_EMPTY;
        }
    }

    // 2. Mark snake body positions in the frame buffer
    for(int i = 0; i < game.snake_length; i++) {
        int x = game.snake_body[i].x;
        int y = game.snake_body[i].y;

        // Head gets a distinct cell type from body segments
        if(i == 0) {
            frame_buffer[y][x] = CELL_SNAKE_HEAD;
        } else {
            frame_buffer[y][x] = CELL_SNAKE_BODY;
        }
    }

    // 3. Mark fruit position in the frame buffer if active
    if(game.fruit_active) {
        frame_buffer[game.fruit.y][game.fruit.x] = CELL_FRUIT;
    }

    // 4. Draw every cell based on its frame buffer entry
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

/**
 *
 * Function: reset_render_flag
 *
 * Deliverable:
 * ------------
 * Reset all frame buffer entries to CELL_EMPTY so the diff
 * comparison in render_game starts from a clean baseline.
 * Called when transitioning from the how to play screen into
 * active gameplay, ensuring no stale cell state from a previous
 * game session causes incorrect rendering.
 *
 * Pseudocode:
 *
 * 1. Loop through every cell in the game grid
 * 2. Set each frame buffer entry to CELL_EMPTY
 *
 **/

void reset_render_flag(void) {

    // 1. Loop through every cell in the game grid
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {

            // 2. Set each frame buffer entry to CELL_EMPTY
            frame_buffer[y][x] = CELL_EMPTY;
        }
    }
}
