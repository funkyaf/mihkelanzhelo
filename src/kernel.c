#include "kernel.h"
#include "modules/utils.h"
#include "modules/char.h"
#include "modules/random.h"

uint32 VGA_INDEX;
static uint32 Y_INDEX = 1;
uint16 cursor_pos = 0, cursor_nextline_index = 1;

/**
 * For QEMU emulation, speed 1 is good
 * For emulating it on VirtualBox, VMware or some other software,
 * change speed value to greater than 4
*/
#define SLEEP_TIME 4

/**
 * colors
 */
uint8 fg_color = WHITE; uint8 bg_color = BLACK;

void change_fg_color_to(uint8 fg) {
    fg_color = fg;
}

void change_bg_color_to(uint8 bg) {
    bg_color = bg;
}

/**
 * VGA stuff
 * printing data onto the screen
*/
uint16 vga_entry(unsigned char ch)
{
    uint16 ax = 0;
    uint8 ah = 0, al = 0;

    ah = bg_color;
    ah <<= 4;
    ah |= fg_color;
    ax = ah;
    ax <<= 8;
    al = ch;
    ax |= al;

    return ax;
}

/**
 * input and output
*/
uint8 inb(uint16 port) {
    uint8 data;
    asm volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

void outb(uint16 port, uint8 data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/**
 * CURSOR
*/
void move_cursor(uint16 pos) {
    outb(0x3D4, 14);
    outb(0x3D5, ((pos >> 8) & 0x00FF));
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0x00FF);
}

void move_cursor_nextline() {
    cursor_pos = 80 * cursor_nextline_index;
    cursor_nextline_index++;
    if (Y_INDEX > 25) {
        cursor_pos = 0; cursor_nextline_index = 1;
    }
    move_cursor(cursor_pos);
}

// postitioning strings and such
void gotoxy(uint16 x, uint16 y)
{
    VGA_INDEX = 80 * y; VGA_INDEX += x;
    if (y > 0) {
        cursor_pos = 80 * cursor_nextline_index * y;
        cursor_nextline_index++;
        move_cursor(cursor_pos);
    }
}

// clear video buffer array
void clear_vga_buffer(uint16 **buffer)
{
    for (uint32 i = 0; i < BUFFER_SIZE; i++) {
        (*buffer)[i] = vga_entry(NULL);
    }
    Y_INDEX = 1; VGA_INDEX = 0;
}

// initializa vga buffer
void init_vga(uint8 foreg_color, uint8 backg_color)
{
    vga_buffer = (uint16*)VGA_ADDRESS; // point vga buffer to vga addr
    clear_vga_buffer(&vga_buffer); // clear buffer
    fg_color = foreg_color; bg_color = backg_color;
    cursor_pos = 0; cursor_nextline_index = 1;
    move_cursor(cursor_pos);
}

char get_input_keycode() {
    char ch = 0;
    while ((ch = inb(KEYBOARD_PORT)) != 0) {
        if (ch > 0) {
            return ch;
        }
    }
    return ch;
}

void wait_for_io(uint32 timer_count) {
    while (1) {
        asm volatile("nop");
        timer_count--;
        if (timer_count <= 0) break;
    }
}

void sleep(uint32 timer_count) {
    wait_for_io(timer_count * 0x02FFFFFF);
}

// internal speaker stuff
void speaker_set(int hz) {
    int divisor = 1193180 / hz;
    outb(PIT_CMDREG, 0xB6);
    outb(PIT_CHANNEL2, (uint8)(divisor));
    outb(PIT_CHANNEL2, (uint8)(divisor >> 8));
}

void beep(uint32 wait_time, uint32 times)
{
    uint8 tempA = inb(0x61);
    uint8 tempB = (inb(0x61) & 0xFC);
    uint32 count;

    for (count = 0; count == times; count++) {
        if (tempA != (tempA | 3)) {
            outb(0x61, tempA | 3);
        }
        sleep(wait_time);
        outb(0x61, tempB); sleep(1);
    }
}

// print a new line
void print_newline() {
    if (Y_INDEX > 25) {
        Y_INDEX = 0; VGA_INDEX = 1;
        clear_vga_buffer(&vga_buffer);
    }
    VGA_INDEX = 80 * Y_INDEX; Y_INDEX++;
    move_cursor_nextline();
}

void print_char(char ch) {
    vga_buffer[VGA_INDEX] = vga_entry(ch);
    VGA_INDEX++;
    move_cursor(++cursor_pos);
}

// print string onto the screen
void print_str(char *str) {
    uint32 i = 0;
    while (str[i]) {
        if ((str[i] == '\n') || (str[i] == '\\')) {
            print_newline(); i++;
        } else {
            print_char(str[i]); i++;
        }
    }
}

void print_int(int num) {
    char str_num[digit_count(num) + 1];
    itoa(num, str_num); print_str(str_num);
}

void test_input() {

    // first init vga with set foreground and background color
    init_vga(WHITE, BLACK);

    speaker_set(1000); beep(1, 2);

    print_str(KERNEL_INFO); print_newline();
    print_str("Number in square brackets indicates the number of color\n\n");
    sleep(7);
    print_str("["); print_int(fg_color); print_str("]> ");

    char ch = 0;
    char keycode = 0;

    do {
        keycode = get_input_keycode();
        if (keycode == KEY_ENTER) {

            uint8 rand_seed = maxrand(664322, 1103515245);
            uint8 rand_color = maxrand(rand_seed, 16);

            if (rand_color > 15) rand_color = 0; //else rand_color++;
            if ((enum vga_color)rand_color == bg_color) rand_color++;
            print_newline(); change_fg_color_to((enum vga_color)rand_color);
            print_str("["); print_int(rand_color); print_str("]> ");
        } else {
            ch = get_ascii_char(keycode);
            print_char(ch);
        }
        sleep(SLEEP_TIME);
    } while (ch > 0);
}

// KERNEL ENTRY
void kernel_entry() {

    init_vga(BRIGHT_GREEN, BLACK);

    print_str("M "); change_fg_color_to(BRIGHT_MAGENTA);
    print_str("I "); change_fg_color_to(BRIGHT_GREEN);
    print_str("H "); change_fg_color_to(BRIGHT_MAGENTA);
    print_str("K "); change_fg_color_to(BRIGHT_GREEN);
    print_str("E "); change_fg_color_to(BRIGHT_MAGENTA);
    print_str("L "); change_fg_color_to(BRIGHT_GREEN);
    print_str("A "); change_fg_color_to(BRIGHT_MAGENTA);
    print_str("N "); change_fg_color_to(BRIGHT_GREEN);
    print_str("Z "); change_fg_color_to(BRIGHT_MAGENTA);
    print_str("H "); change_fg_color_to(BRIGHT_GREEN);
    print_str("E "); change_fg_color_to(BRIGHT_MAGENTA);
    print_str("L "); change_fg_color_to(BRIGHT_GREEN);
    print_str("O "); print_newline(); change_fg_color_to(WHITE);
    print_str("=================================="); print_newline();

    sleep(10);
    print_str("I see you're using my OS :)"); sleep(4);
    test_input();
}
