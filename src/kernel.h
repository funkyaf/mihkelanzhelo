#ifndef KERNEL_H
#define KERNEL_H

char *KERNEL_INFO = "Mihkelanzhelo 2020 v0.2.4 (with some RNG)";

#include "modules/types.h"
#include "modules/keyboard.h"

#define NULL 0

#define VGA_ADDRESS 0xB8000
#define BUFFER_SIZE 2200

uint16* vga_buffer;

enum vga_color {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    GREY,
    DARK_GREY,
    BRIGHT_BLUE,
    BRIGHT_GREEN,
    BRIGHT_CYAN,
    BRIGHT_RED,
    BRIGHT_MAGENTA,
    YELLOW,
    WHITE,
};

// internal speaker stuff
#define PIT_CHANNEL2 0x42
#define PIT_CMDREG 0x43 

#endif