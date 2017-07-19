#include <stddef.h>

// Because everyone loves ints and bools.
#include <stdint.h>
#include <stdbool.h>
 
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
#if !defined(__i386__)
#error "Currently this is only supposed to be compiled with a 32 bit compiler"
#endif
 
static inline uint8_t vga_entry_color(int fg, int bg) {
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len]) {
		len++;
	}
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void) {
	// All text starts at the top of the screen.
	terminal_row = 0;
	terminal_column = 0;

	// Set terminal colors, 7 is grey and 0 is black.
	terminal_color = vga_entry_color(7, 0);

	terminal_buffer = (uint16_t*) 0xB8000;

	// This goes throught the buffer and changes the color of every item in it.
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_putchar(char c) {
	// Put the char at the desired location in the buffer.
	const size_t index = terminal_row * VGA_WIDTH + terminal_column;
	terminal_buffer[index] = vga_entry(c, terminal_color);

	// Make sure things dont go out of the screen.
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT) {
			terminal_row = 0;
		}
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		terminal_putchar(data[i]);
	}
}

void terminal_scroll() {
	// Replace every item in the buffer with the item before it.
	for (size_t y = 0; y < VGA_HEIGHT - 1; ++y) {
		for (size_t x = 0; x < VGA_WIDTH; ++x) {
			const size_t index = y * VGA_WIDTH + x;
			const size_t index2 = (y + 1) * VGA_WIDTH + x;
			terminal_buffer[index] = terminal_buffer[index2];
		}
	}

	for (size_t x = 0; x < VGA_WIDTH; ++x) {
		const size_t index = VGA_WIDTH * (VGA_HEIGHT - 1) + x;
		terminal_buffer[index] = vga_entry(' ', 7);
	}

	--terminal_row;
}
 
void terminal_puts(const char* data) {
	/* reset the column so text doesnt 
									   go
									      like 
									           this */
	terminal_column = 0;

	terminal_write(data, strlen(data));

	if (terminal_row == VGA_HEIGHT) {
		terminal_scroll();
	}

	terminal_row = terminal_row + 1;
}
 
void kernel_main(void) {
	// Initialize terminal interface.
	terminal_initialize();

	// Newlines do not exist yet. Bring them into being!
	terminal_puts("Hello, kernel World!\n");
	terminal_puts("test");

	// kernel_main exits and the machine goes into an endless loop
}