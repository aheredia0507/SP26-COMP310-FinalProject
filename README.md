# Snake Arcade — Bare Metal i386 Snake Game

A fully functional Snake game running directly on bare metal x86 hardware
with no operating system.

The game boots via GRUB, runs in 32-bit protected mode on an i386 CPU,
and demonstrates operating system concepts including memory-mapped I/O,
interrupt-driven hardware timers, polling I/O, and pseudo-random number
generation without a standard library.

---

## How to Build and Run

**Requirements:**
- i686-pc-linux-gnu cross compiler toolchain
- GRUB i386-pc
- QEMU i386
- mtools

**Build:**
```bash
make
```

**Run:**
```bash
make run
```

---

## How to Play

- Press Enter on the title screen to start
- Use arrow keys to move the snake
- Eat the red fruit to grow
- Avoid the walls and yourself
- Fill the entire screen to win
- Your fruit count is displayed on the game over screen

---

## OS Concepts Demonstrated

**Boot Process (OS Book Chapter 2)**
The kernel is loaded by GRUB using a Multiboot2 header embedded in the
binary. The linker script places the multiboot header at address 0 so
GRUB can find it. GRUB sets up a 1024x768 32-bit framebuffer and passes
its address to the kernel via the multiboot2 info structure.

**Memory-Mapped I/O (OS Book Section 3.2.1)**
All rendering writes directly to the framebuffer base address provided
by GRUB. Every pixel drawn — the snake, apple sprite, bitmap font, and
score — is a direct memory write to a hardware-mapped address. No display
driver or graphics library is involved.

**Polling I/O (OS Book Section 3.3)**
Keyboard input is handled by polling the PS/2 controller status register
at port 0x64 on every loop iteration and reading scancodes from port 0x60
when data is available. This matches Exercise 3.1 from the OS book.

**Interrupt-Driven I/O (OS Book Section 3.4)**
Snake movement is driven by a PIT (Programmable Interval Timer) hardware
interrupt rather than a busy-wait frame counter. The PIT fires at ~18 Hz
via IRQ 0. An ISR registered at IDT vector 32 sets a timer flag that the
game loop checks to advance the snake. The IDT, GDT, and PIC setup is
adapted from HW3.

**Random Number Generation**
Generating random numbers on deterministic hardware requires two components.
An entropy source provides unpredictability — the PIT counter is read at
the exact CPU cycle the player presses Enter, making the value dependent
on human reaction time. A Linear Congruential Generator (LCG) uses that
seed to produce the fruit spawn sequence. The same seed always produces
the same sequence, but the seed itself is never the same twice.

**Freestanding Kernel Environment (OS Book Section 2.3.3)**
The kernel is compiled with -ffreestanding meaning no standard library
is available. printf, malloc, itoa, and all libc functions are absent.
The bitmap font renderer, integer-to-digit converter, and all string
drawing functions are implemented from scratch as a direct consequence
of this constraint.

---
## Technical Details

- **Architecture:** i386 32-bit protected mode
- **Bootloader:** GRUB with Multiboot2
- **Display:** 1024x768 32-bit pixel framebuffer
- **Grid:** 12x9 tiles, each tile 60x60 pixels
- **Snake speed:** ~6 moves per second (PIT at 18 Hz, threshold 3)
- **Max score:** 105 fruits (grid size 108, starting length 3)
- **RNG:** LCG seeded from PIT counter at player's first keypress
- **Cross compiler:** i686-pc-linux-gnu-gcc with -ffreestanding
