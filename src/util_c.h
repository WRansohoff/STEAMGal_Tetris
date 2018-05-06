#ifndef _VVC_UTIL_C_H
#define _VVC_UTIL_C_H

#include "global.h"
#include "peripherals.h"

// C-languages utility method signatures.

// Methods for interacting with specific I2C devices.
void ssd1306_start_sequence(I2C_TypeDef *I2Cx);

// Methods for writing to the 1KB OLED framebuffer.
// These don't actually write through to the screen until
// the 'i2c_display_framebuffer' method is called (see util.S)
void oled_draw_h_line(int x, int y, int w, unsigned char color);
void oled_draw_v_line(int x, int y, int h, unsigned char color);
void oled_draw_rect(int x, int y, int w, int h,
                    int outline, unsigned char color);
void oled_write_pixel(int x, int y, unsigned char color);
void oled_draw_letter(int x, int y, unsigned int w0, unsigned int w1, unsigned char color, char size);
void oled_draw_letter_c(int x, int y, char c, unsigned char color, char size);
void oled_draw_letter_i(int x, int y, int ic, unsigned char color, char size);
void oled_draw_text(int x, int y, char* cc, unsigned char color, char size);

// Tetris methods!
void draw_main_menu(void);
void draw_game_over(void);
void draw_tetris_game(void);
void reset_game_state(void);
void tetris_game_tick(void);

#endif
