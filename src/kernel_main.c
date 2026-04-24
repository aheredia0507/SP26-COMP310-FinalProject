/**
 *
 * Final Project - Snake Game
 *
 * Main kernel entry point. Parses multiboot2 info, sets up the
 * framebuffer, provides low-level port I/O, and reads the PIT
 * counter to supply entropy for the random number generator.
 *
 **/

#include <stdint.h>
#include "snake.h"
#include "game.h"
#include "interrupt.h"

// Multiboot constants
#define INFO_TYPE_FRAMEBUFFER_INFO 8
#define INFO_TYPE_DONE 0
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6
#define MULTIBOOT2_HEADER_VIDINFO_TAG 5

// Multiboot structures
struct multiboot_header {
    uint32_t magic;
    uint32_t flags;
    uint32_t length;
    uint32_t checksum;
};

struct multiboot_framebuffer_tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t x;
};

struct multiboot_tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
};

// Multiboot headers
struct multiboot_header mbh __attribute__((section(".multiboot1"))) = {
    .magic    = MULTIBOOT2_HEADER_MAGIC,
    .flags    = 0,
    .length   = 16,
    .checksum = -(16 + MULTIBOOT2_HEADER_MAGIC)
};

struct multiboot_framebuffer_tag gfxtag __attribute__((section(".multiboot2"))) = {
    .type   = MULTIBOOT2_HEADER_VIDINFO_TAG,
    .flags  = 1,
    .size   = sizeof(struct multiboot_framebuffer_tag),
    .width  = 1024,
    .height = 768,
    .depth  = 32
};

struct multiboot_tag terminator_tag __attribute__((section(".multiboot3"))) = {
    .type  = 0,
    .flags = 0,
    .size  = sizeof(struct multiboot_tag)
};

/**
 *
 * Function: inb
 *
 * Deliverable:
 * ------------
 * Read one byte from an x86 I/O port using inline assembly.
 * Used to poll the PS/2 keyboard controller and the PIT.
 *
 * @param _port - The 16-bit I/O port address to read from
 *
 * Pseudocode:
 *
 * 1. Execute the x86 inb instruction targeting _port
 * 2. Return the byte read from the port
 *
 **/

uint8_t inb(uint16_t _port) {
    uint8_t rv;

    // 1-2. Read one byte from the port using inline assembly
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));

    return rv;
}

/**
 *
 * Function: outb
 *
 * Deliverable:
 * ------------
 * Write one byte to an x86 I/O port using inline assembly.
 * Required to send the PIT latch command before reading the counter.
 * Matches the outb implementation from the OS book (Listing 12).
 *
 * @param _port - The 16-bit I/O port address to write to
 * @param _data - The byte value to write
 *
 * Pseudocode:
 *
 * 1. Execute the x86 out instruction sending _data to _port
 *
 **/

void outb(uint16_t _port, uint8_t _data) {

    // 1. Write one byte to the port using inline assembly
    __asm__ __volatile__ ("out %1, %0" : : "dN" (_port), "a" (_data));
}

/**
 *
 * Function: read_pit_count
 *
 * Deliverable:
 * ------------
 * Read the current counter value from PIT channel 0 and return it
 * as an entropy source for seeding the random number generator.
 *
 * The PIT (Programmable Interval Timer) increments continuously since
 * boot. Because we call this at the exact moment the player presses
 * Enter, the counter value depends on unpredictable human timing,
 * giving us a non-deterministic seed even though the CPU is deterministic.
 *
 * PIT register map (OS book Table 3.1):
 *   0x40 - Channel 0 data (read counter here)
 *   0x43 - Mode/Command register (send latch command here)
 *
 * Pseudocode:
 *
 * 1. Send latch command (0x00) to PIT mode register (0x43)
 *    to freeze the current count for a stable two-byte read
 * 2. Read the low byte of the counter from channel 0 (0x40)
 * 3. Read the high byte of the counter from channel 0 (0x40)
 * 4. Combine bytes and return the 16-bit counter value
 *
 **/

uint16_t read_pit_count(void) {
    uint16_t count;

    // 1. Send latch command to PIT mode register so count is stable
    outb(0x43, 0x00);

    // 2. Read low byte from PIT channel 0
    count = inb(0x40);

    // 3. Read high byte from PIT channel 0 and shift into place
    count |= (uint16_t)inb(0x40) << 8;

    // 4. Return the full 16-bit counter value
    return count;
}

// Framebuffer globals
uint32_t *framebuffer;
static uint32_t framebufferWidth;
static uint32_t framebufferHeight;

// Framebuffer setters - called by parseMultiboot2Info to store
// the base address and dimensions returned in the multiboot2 tag
void setFramebufferAddress(void *base) { framebuffer = base; }
void setFramebufferWidth(uint32_t width) { framebufferWidth = width; }
void setFramebufferHeight(uint32_t height) { framebufferHeight = height; }

uint32_t *pMultibootInfo;

/**
 *
 * Function: parseMultiboot2Info
 *
 * Deliverable:
 * ------------
 * Walk the multiboot2 info structure passed by GRUB and extract
 * the framebuffer base address and dimensions.
 *
 * Pseudocode:
 *
 * 1. Point p at the first tag (skip the 8-byte header)
 * 2. Loop through tags until the terminating tag (type 0) is found
 * 3. If a framebuffer tag is found, store address, width, and height
 * 4. Advance p past the current tag (aligned to 8 bytes)
 *
 **/

int parseMultiboot2Info(void) {
    uint32_t *p = pMultibootInfo + 2;
    unsigned int totalStructSize = *pMultibootInfo;

    // 1-2. Walk tags until terminator
    while(p < (uint32_t*)((uint8_t*)pMultibootInfo + totalStructSize)) {
        unsigned int type = *p;
        unsigned int size = *(p + 1);

        // 3. Extract framebuffer info when found
        if(type == INFO_TYPE_FRAMEBUFFER_INFO) {
            setFramebufferAddress((void*)*(p + 2));
            setFramebufferWidth(*(p + 5));
            setFramebufferHeight(*(p + 6));
        }

        if(type == INFO_TYPE_DONE) return 0;

        // 4. Advance past current tag (size must be 8-byte aligned)
        while(((uint32_t)size & ~7) != (uint32_t)size) size++;
        p += (size / sizeof(unsigned int));
    }

    return 0;
}

/**
 *
 * Function: drawPixel
 *
 * Deliverable:
 * ------------
 * Write a 32-bit color value to a single pixel in the framebuffer.
 * Implements memory-mapped I/O to the display (OS book Section 3.2.1).
 *
 * @param x     - Horizontal pixel coordinate
 * @param y     - Vertical pixel coordinate
 * @param color - 32-bit RGB color value
 *
 * Pseudocode:
 *
 * 1. Calculate the pixel's index in the framebuffer array
 * 2. Write the color value to that index
 *
 **/

void drawPixel(int x, int y, int color) {
    // 1-2. Write color to the framebuffer at the computed offset
    framebuffer[x + (y * framebufferWidth)] = (uint32_t)color;
}

/**
 *
 * Function: program_pit_timer
 *
 * Deliverable:
 * ------------
 * Configure PIT channel 0 to fire interrupts at a fixed frequency.
 * The snake moves once per interrupt, so this controls game speed.
 *
 * PIT base clock is 1,193,182 Hz. Frequency = 1193182 / reload_val.
 * reload_val of 65535 gives approximately 18 Hz base rate.
 * Final snake speed is divided down further by the threshold
 * in pit_handler (interrupt.c)
 *
 * Reference: OS book Table 3.1 and Listing 13 (programPIT function)
 *
 * @param reload_val - Counter reload value (determines tick frequency)
 *
 * Pseudocode:
 *
 * 1. Send mode command to PIT (channel 0, lobyte/hibyte, rate generator)
 * 2. Send low byte of reload value to channel 0 data port
 * 3. Send high byte of reload value to channel 0 data port
 *
 **/

void program_pit_timer(uint32_t reload_val) {

    // 1. Mode command: channel 0, lobyte/hibyte, rate generator (0x34)
    outb(0x43, 0x34);

    // 2. Send low byte of reload value
    outb(0x40, reload_val & 0xFF);

    // 3. Send high byte of reload value
    outb(0x40, (reload_val >> 8) & 0xFF);
}

/**
 *
 * Function: main
 *
 * Deliverable:
 * ------------
 * Kernel entry point. Capture the multiboot info pointer from EBX,
 * parse it to find the framebuffer, clear the screen, then hand
 * control to the game loop.
 *
 * Pseudocode:
 *
 * 1. Capture multiboot info pointer from EBX register
 * 2. Parse multiboot2 info to find framebuffer address and dimensions
 * 3. Clear screen to black
 * 4. Remap PIC so IRQs land at vectors 32-47
 * 5. Load GDT for protected mode
 * 6. Install PIT handler in IDT at vector 32
 * 7. Program PIT to fire at 5 Hz (snake movement speed)
 * 8. Enable interrupts with sti
 * 9. Start the game loop
 * 10. Spin forever (should never reach here)
 *
 **/

void main(void) {

    // 1. Capture multiboot info pointer passed by GRUB in EBX
    asm("mov %%ebx,%0" : "=r"(pMultibootInfo) : :);

    // 2. Parse multiboot2 tags to set up the framebuffer
    parseMultiboot2Info();

    // 3. Clear screen to black
    for(int x = 0; x < framebufferWidth; x++) {
        for(int y = 0; y < framebufferHeight; y++) {
            drawPixel(x, y, 0x000000);
        }
    }

    // 4. Remap PIC so hardware IRQs don't collide with CPU exceptions
    remap_pic();

    // 5. Load GDT for protected mode segment descriptors
    load_gdt();

    // 6. Install PIT handler in IDT at vector 32
    init_idt();

    // 7. Program PIT to fire at max slow rate (~18 Hz base)
    //    1193182 / 65535 = ~18 Hz, divided down by pit_handler threshold
    program_pit_timer(65535);

    // 8. Enable interrupts - PIT will now fire and drive game movement
    asm("sti");

    // 9. Start the game loop
    game_loop();

    // 10. Spin forever (should never reach here)
    while(1) {}
}
