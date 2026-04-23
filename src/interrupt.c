/**
 *
 * Arthur Heredia - COMP 310 - Operating Systems
 *
 * Final Project - Snake Game - Interrupt Module
 *
 * Stripped-down interrupt infrastructure adapted from HW3 interrupt.c.
 * Sets up the GDT, IDT, and PIC so the PIT can fire timer interrupts
 * that drive snake movement instead of a busy-wait frame counter.
 *
 * What was kept from HW3:
 *   - remap_pic()    - remaps PIC so IRQs don't collide with CPU exceptions
 *   - load_gdt()     - loads the global descriptor table for protected mode
 *   - init_idt()     - installs the PIT handler at IDT vector 32
 *   - PIC_sendEOI()  - signals end of interrupt so PIC fires future IRQs
 *   - pit_handler()  - was a stub in HW3, now sets timer_tick for game loop
 *
 * What was removed from HW3:
 *   - All exception handlers (divide error, page fault, etc.)
 *   - keyboard_handler (game polls keyboard manually via read_keyboard_input)
 *   - syscall_handler
 *   - write_tss / tss_flush (no task switching needed)
 *   - memset (replaced with inline loop)
 *
 **/

#include "interrupt.h"

// inb and outb are defined in kernel_main.c
extern uint8_t inb(uint16_t port);
extern void    outb(uint16_t port, uint8_t data);

// IDT and GDT globals
static struct idt_entry idt_entries[IDT_SIZE];
static struct idt_ptr   idt_ptr;

// Timer tick flag - set by pit_handler, checked by game_loop in game.c
volatile int timer_tick = 0;

// GDT: null descriptor, kernel code, kernel data
// Identical to HW3 entries 0-2 - provides protected mode memory segments
static struct gdt_entry_bits gdt[] = {
    {   // Entry 0: null descriptor (required by x86 spec)
        .limit_low              = 0,
        .base_low               = 0,
        .accessed               = 0,
        .read_write             = 0,
        .conforming_expand_down = 0,
        .code                   = 0,
        .always_1               = 0,
        .DPL                    = 0,
        .present                = 0,
        .limit_high             = 0,
        .available              = 0,
        .always_0               = 0,
        .big                    = 0,
        .gran                   = 0,
        .base_high              = 0
    },
    {   // Entry 1: kernel code segment (selector 0x08)
        .limit_low              = 0xffff,
        .base_low               = 0,
        .accessed               = 0,
        .read_write             = 1,
        .conforming_expand_down = 0,
        .code                   = 1,
        .always_1               = 1,
        .DPL                    = 0,
        .present                = 1,
        .limit_high             = 0xf,
        .available              = 0,
        .always_0               = 0,
        .big                    = 1,
        .gran                   = 1,
        .base_high              = 0
    },
    {   // Entry 2: kernel data segment (selector 0x10)
        .limit_low              = 0xffff,
        .base_low               = 0,
        .accessed               = 0,
        .read_write             = 1,
        .conforming_expand_down = 0,
        .code                   = 0,
        .always_1               = 1,
        .DPL                    = 0,
        .present                = 1,
        .limit_high             = 0xf,
        .available              = 0,
        .always_0               = 0,
        .big                    = 1,
        .gran                   = 1,
        .base_high              = 0
    }
};

struct seg_desc gdt_desc = {
    .sz   = sizeof(gdt) - 1,
    .addr = (uint32_t)(&gdt[0])
};

/**
 *
 * Function: PIC_sendEOI
 *
 * Deliverable:
 * ------------
 * Send End of Interrupt signal to the PIC so it can fire future interrupts.
 * Must be called at the end of every ISR or the PIC will stop sending IRQs.
 * Adapted directly from HW3 interrupt.c.
 *
 * @param irq - IRQ number that was handled (0 for PIT)
 *
 * Pseudocode:
 *
 * 1. If IRQ came from slave PIC (IRQ >= 8), send EOI to slave PIC
 * 2. Always send EOI to master PIC
 *
 **/
void PIC_sendEOI(unsigned char irq) {

    // 1. If IRQ is from slave PIC, send EOI there first
    if(irq >= 8) {
        outb(PIC_2_CTRL, PIC_EOI);
    }

    // 2. Always send EOI to master PIC
    outb(PIC_1_CTRL, PIC_EOI);
}

/**
 *
 * Function: pit_handler
 *
 * Deliverable:
 * ------------
 * Interrupt Service Routine for PIT channel 0 (IRQ 0, IDT vector 32).
 * Called automatically by the CPU each time the PIT counter reaches zero.
 *
 * Uses a tick counter to divide the PIT frequency down to a playable
 * snake speed. The PIT fires at 1193182 / 65535 ≈ 18 Hz. With a
 * threshold of 30, the snake moves at 18 / 30 ≈ 0.6 times per second.
 * Increase the threshold to make the snake slower.
 *
 * The __attribute__((interrupt)) tells gcc to generate the proper ISR
 * prologue and epilogue - it saves all registers before our code runs
 * and restores them after, so the interrupted program is not disrupted.
 * This is the behavior described in OS book Section 3.4.
 *
 * Pseudocode:
 *
 * 1. Increment tick counter each time PIT fires
 * 2. Check if tick counter has reached the threshold
 * 3. If threshold reached, set timer_tick flag and reset counter
 * 4. Send End of Interrupt to PIC so future timer interrupts can fire
 *
 **/
__attribute__((interrupt)) void pit_handler(struct interrupt_frame *frame) {

    // 1. Increment tick counter each time PIT fires
    static int tick_count = 0;
    tick_count++;

    // 2-3. Only signal the game every N interrupts to control snake speed
    //      Increase threshold to make snake slower, decrease to make faster
    if(tick_count >= 3) {
        timer_tick = 1;
        tick_count = 0;
    }

    // 4. Send EOI to master PIC (IRQ 0 is on master PIC)
    PIC_sendEOI(0);
}

/**
 *
 * Function: load_gdt
 *
 * Deliverable:
 * ------------
 * Load the Global Descriptor Table into the CPU using the lgdt instruction.
 * Required before interrupts can work in 32-bit protected mode.
 * Adapted directly from HW3 interrupt.c.
 *
 * Pseudocode:
 *
 * 1. Disable interrupts
 * 2. Load GDT pointer with lgdt instruction
 * 3. Far jump to flush the code segment register (CS)
 * 4. Reload all data segment registers with kernel data selector (0x10)
 *
 **/
void load_gdt(void) {

    // 1-4. Load GDT and reload segment registers via inline assembly
    asm("cli\n"
        "lgdt [gdt_desc]\n"
        "ljmp $0x8,$gdt_flush\n"
"gdt_flush:\n"
        "mov %%eax, 0x10\n"
        "mov %%ds, %%eax\n"
        "mov %%ss, %%eax\n"
        "mov %%es, %%eax\n"
        "mov %%fs, %%eax\n"
        "mov %%gs, %%eax\n" : : : "eax");
}

/**
 *
 * Function: idt_set_gate
 *
 * Deliverable:
 * ------------
 * Install one ISR into the IDT at a given vector number.
 * Adapted directly from HW3 interrupt.c.
 *
 * @param num   - IDT vector number (32 = IRQ0 PIT timer)
 * @param base  - Address of the ISR function
 * @param sel   - Code segment selector (0x08 = kernel code)
 * @param flags - Gate type flags (0x8E = present, ring 0, interrupt gate)
 *
 * Pseudocode:
 *
 * 1. Store low 16 bits of handler address
 * 2. Store segment selector
 * 3. Zero the reserved byte
 * 4. Store flags byte
 * 5. Store high 16 bits of handler address
 *
 **/
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {

    // 1. Low 16 bits of handler address
    idt_entries[num].base_lo = base & 0xFFFF;

    // 2. Segment selector
    idt_entries[num].sel     = sel;

    // 3. Reserved byte must always be zero
    idt_entries[num].always0 = 0;

    // 4. Flags (present bit, privilege level, gate type)
    idt_entries[num].flags   = flags;

    // 5. High 16 bits of handler address
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
}

/**
 *
 * Function: idt_flush
 *
 * Deliverable:
 * ------------
 * Load the IDT pointer into the CPU using the lidt instruction.
 * Adapted directly from HW3 interrupt.c.
 *
 * @param idt - Pointer to the idt_ptr structure to load
 *
 * Pseudocode:
 *
 * 1. Execute lidt instruction with the IDT pointer
 *
 **/
static void idt_flush(struct idt_ptr *idt) {

    // 1. Load IDT into CPU
    asm("lidt %0\n" : : "m"(*idt) :);
}

/**
 *
 * Function: init_idt
 *
 * Deliverable:
 * ------------
 * Initialize the IDT and install the PIT timer handler at vector 32.
 *
 * Stripped down from HW3 init_idt() which registered handlers for all
 * 256 vectors including exceptions, keyboard, and syscall. The snake
 * game only needs the PIT timer at vector 32. All other vectors are
 * zeroed - if an unexpected interrupt fires the CPU will triple fault,
 * which is acceptable for a single-purpose bare metal game.
 *
 * Pseudocode:
 *
 * 1. Set IDT pointer limit and base address
 * 2. Zero all 256 IDT entries
 * 3. Install pit_handler at vector 32 (IRQ 0 maps to vector 32
 *    after PIC remapping in remap_pic)
 * 4. Load the IDT into the CPU with idt_flush
 *
 **/
void init_idt(void) {

    // 1. Set IDT pointer
    idt_ptr.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // 2. Zero all IDT entries
    for(int i = 0; i < IDT_SIZE; i++) {
        idt_entries[i].base_lo = 0;
        idt_entries[i].sel     = 0;
        idt_entries[i].always0 = 0;
        idt_entries[i].flags   = 0;
        idt_entries[i].base_hi = 0;
    }

    // 3. Install PIT handler at vector 32 (IRQ 0 after PIC remap)
    idt_set_gate(32, (uint32_t)pit_handler, 0x08, 0x8E);

    // 4. Load IDT into CPU
    idt_flush(&idt_ptr);
}

/**
 *
 * Function: remap_pic
 *
 * Deliverable:
 * ------------
 * Remap the PIC so hardware IRQs land at vectors 32-47 instead of
 * 0-15. This is required because vectors 0-31 are reserved by Intel
 * for CPU exceptions - without remapping, IRQ 0 (PIT) would collide
 * with the divide-by-zero exception handler at vector 0.
 * Adapted directly from HW3 interrupt.c.
 *
 * Pseudocode:
 *
 * 1. Send initialization command to both PICs (ICW1)
 * 2. Remap master PIC to start at vector 32 (ICW2)
 * 3. Remap slave PIC to start at vector 40 (ICW2)
 * 4. Set up cascading between master and slave (ICW3)
 * 5. Set 8086 mode on both PICs (ICW4)
 * 6. Mask all IRQs except IRQ 0 (PIT timer)
 *
 **/
void remap_pic(void) {

    // 1. Begin initialization sequence on both PICs
    outb(PIC_1_CTRL, 0x11);
    outb(PIC_2_CTRL, 0x11);

    // 2. Remap master PIC - IRQ 0 now maps to IDT vector 32
    outb(PIC_1_DATA, 0x20);

    // 3. Remap slave PIC - IRQ 8 now maps to IDT vector 40
    outb(PIC_2_DATA, 0x28);

    // 4. Set up cascading (slave connected to master IRQ 2)
    outb(PIC_1_DATA, 0x00);
    outb(PIC_2_DATA, 0x00);

    // 5. Set 8086 mode on both PICs
    outb(PIC_1_DATA, 0x01);
    outb(PIC_2_DATA, 0x01);

    // 6. Mask all IRQs then unmask only IRQ 0 (PIT timer)
    //    0xFE = 11111110 in binary - only bit 0 (IRQ 0) is unmasked
    outb(PIC_1_DATA, 0xFE);
    outb(PIC_2_DATA, 0xFF);
}
