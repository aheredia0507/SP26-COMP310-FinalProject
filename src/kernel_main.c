/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game
 *
 * Main kernel entry point
 *
 **/

#include <stdint.h>
#include "snake.h"
#include "game.h"

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
    .magic = MULTIBOOT2_HEADER_MAGIC,
    .flags = 0,
    .length = 16,
    .checksum = -(16 + MULTIBOOT2_HEADER_MAGIC)
};

struct multiboot_framebuffer_tag gfxtag __attribute__((section(".multiboot2"))) = {
    .type = MULTIBOOT2_HEADER_VIDINFO_TAG,
    .flags = 1,
    .size = sizeof(struct multiboot_framebuffer_tag),
    .width = 1024,
    .height = 768,
    .depth = 32
};

struct multiboot_tag terminator_tag __attribute__((section(".multiboot3"))) = {
    .type = 0,
    .flags = 0,
    .size = sizeof(struct multiboot_tag)
};

// Port I/O
uint8_t inb(uint16_t _port) {
    uint8_t rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

// Framebuffer globals
uint32_t *framebuffer;
static uint32_t framebufferWidth;
static uint32_t framebufferHeight;

void setFramebufferAddress(void *base) { framebuffer = base; }
void setFramebufferWidth(uint32_t width) { framebufferWidth = width; }
void setFramebufferHeight(uint32_t height) { framebufferHeight = height; }

uint32_t *pMultibootInfo;

int parseMultiboot2Info(void) {
    uint32_t *p = pMultibootInfo + 2;
    unsigned int totalStructSize = *pMultibootInfo;

    while(p < (uint32_t*)((uint8_t*)pMultibootInfo + totalStructSize)) {
        unsigned int type = *p;
        unsigned int size = *(p + 1);
        
        if(type == INFO_TYPE_FRAMEBUFFER_INFO) {
            setFramebufferAddress((void*)*(p + 2));
            setFramebufferWidth(*(p + 5));
            setFramebufferHeight(*(p + 6));
        }
        
        if(type == INFO_TYPE_DONE) return 0;
        
        while(((uint32_t)size & ~7) != (uint32_t)size) size++;
        p += (size / sizeof(unsigned int));
    }
    
    return 0;
}

void drawPixel(int x, int y, int color) {
    framebuffer[x + (y * framebufferWidth)] = (uint32_t)color;
}

void main(void) {
    asm("mov %%ebx,%0" : "=r"(pMultibootInfo) : :);
    parseMultiboot2Info();
    
    // Clear screen to black
    for(int x = 0; x < framebufferWidth; x++) {
        for(int y = 0; y < framebufferHeight; y++) {
            drawPixel(x, y, 0x000000);
        }
    }
    
    // Start game
    game_loop();
    
    while(1) {}
}
