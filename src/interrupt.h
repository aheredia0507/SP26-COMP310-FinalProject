/**
 *
 * Final Project - Snake Game - Interrupt Module
 *
 * Stripped-down interrupt infrastructure for the snake game.
 * Adapted from HW3 interrupt.h - contains only what is needed
 * to set up the IDT and run the PIT timer interrupt.
 *
 * Removed from HW3 original:
 *   - TSS/task switching structures (not needed for single program)
 *   - eflags struct (not referenced in game)
 *   - Keyboard/syscall handler declarations (game polls keyboard manually)
 *
 **/

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

// PIC port addresses
#define PIC_EOI       0x20   // End-of-interrupt command code
#define PIC_1_CTRL    0x20   // Master PIC command port
#define PIC_2_CTRL    0xA0   // Slave PIC command port
#define PIC_1_DATA    0x21   // Master PIC data port
#define PIC_2_DATA    0xA1   // Slave PIC data port

// IDT size
#define IDT_SIZE 256

// IDT entry - one slot in the interrupt descriptor table
// Tells the CPU where to jump when a given interrupt fires
struct idt_entry {
    uint16_t base_lo;    // Low 16 bits of handler address
    uint16_t sel;        // Kernel segment selector (0x08)
    uint8_t  always0;    // Reserved, must be zero
    uint8_t  flags;      // Gate type and privilege flags
    uint16_t base_hi;    // High 16 bits of handler address
} __attribute__((packed));

// IDT pointer - passed to the lidt instruction to load the IDT
struct idt_ptr {
    uint16_t limit;      // Size of IDT in bytes minus 1
    uint32_t base;       // Address of first idt_entry
} __attribute__((packed));

// Interrupt stack frame - pushed automatically by CPU before calling ISR
struct interrupt_frame {
    uint32_t eip;
    uint16_t cs;
    uint16_t unused1;
    uint32_t eflags;
    uint32_t esp;
    uint16_t ss;
    uint16_t unused2;
} __attribute__((packed));

// GDT entry - defines a memory segment for protected mode
struct gdt_entry_bits {
    unsigned int limit_low              : 16;
    unsigned int base_low               : 24;
    unsigned int accessed               : 1;
    unsigned int read_write             : 1;
    unsigned int conforming_expand_down : 1;
    unsigned int code                   : 1;
    unsigned int always_1               : 1;
    unsigned int DPL                    : 2;
    unsigned int present                : 1;
    unsigned int limit_high             : 4;
    unsigned int available              : 1;
    unsigned int always_0               : 1;
    unsigned int big                    : 1;
    unsigned int gran                   : 1;
    unsigned int base_high              : 8;
} __attribute__((packed));

// GDT pointer - passed to the lgdt instruction
struct seg_desc {
    uint16_t sz;
    uint32_t addr;
} __attribute__((packed));

// Timer tick flag - set by pit_handler, read by game_loop
extern volatile int timer_tick;

// Function declarations
void remap_pic(void);
void load_gdt(void);
void init_idt(void);
void PIC_sendEOI(unsigned char irq);

#endif
