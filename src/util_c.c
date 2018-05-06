#include "util_c.h"

// C-language utility method definitions.
/*
 * Send a series of startup commands over I2C.
 */
void ssd1306_start_sequence(I2C_TypeDef *I2Cx) {
  // Display clock division
  i2c_write_command(I2Cx, 0xD5);
  i2c_write_command(I2Cx, 0x80);
  // Set multiplex
  i2c_write_command(I2Cx, 0xA8);
  i2c_write_command(I2Cx, 0x3F);
  // Set display offset ('start column')
  i2c_write_command(I2Cx, 0xD3);
  i2c_write_command(I2Cx, 0x00);
  // Set start line (0b01000000 | line)
  i2c_write_command(I2Cx, 0x40);
  // Set internal charge pump (on)
  i2c_write_command(I2Cx, 0x8D);
  i2c_write_command(I2Cx, 0x14);
  // Set memory mode
  i2c_write_command(I2Cx, 0x20);
  i2c_write_command(I2Cx, 0x00);
  // Set 'SEGREMAP'
  i2c_write_command(I2Cx, 0xA1);
  // Set column scan (descending)
  i2c_write_command(I2Cx, 0xC8);
  // Set 'COMPINS'
  i2c_write_command(I2Cx, 0xDA);
  i2c_write_command(I2Cx, 0x12);
  // Set contrast
  i2c_write_command(I2Cx, 0x81);
  i2c_write_command(I2Cx, 0xCF);
  // Set precharge
  i2c_write_command(I2Cx, 0xD9);
  i2c_write_command(I2Cx, 0xF1);
  // Set VCOM detect
  i2c_write_command(I2Cx, 0xDB);
  i2c_write_command(I2Cx, 0x40);
  // Set output to follow RAM content
  i2c_write_command(I2Cx, 0xA4);
  // Normal display mode
  i2c_write_command(I2Cx, 0xA6);
  // Display on
  i2c_write_command(I2Cx, 0xAF);
}

/*
 * Draw a horizontal line.
 * First, calculate the Y bitmask and byte offset, then just go from x->x.
 */
inline void oled_draw_h_line(int x, int y,
                             int w, unsigned char color) {
  int y_page_offset = y / 8;
  y_page_offset *= 128;
  int bit_to_set = 0x01 << (y & 0x07);
  if (!color) {
    bit_to_set = ~bit_to_set;
  }
  int x_pos;
  for (x_pos = x; x_pos < (x+w); ++x_pos) {
    if (color) {
      oled_fb[x_pos + y_page_offset] |= bit_to_set;
    }
    else {
      oled_fb[x_pos + y_page_offset] &= bit_to_set;
    }
  }
}

/*
 * Draw a veritcal line.
 */
inline void oled_draw_v_line(int x, int y,
                             int h, unsigned char color) {
  int y_page_offset;
  int bit_to_set;
  int y_pos;
  for (y_pos = y; y_pos < (y+h); ++y_pos) {
    y_page_offset = y_pos/8;
    y_page_offset *= 128;
    bit_to_set = 0x01 << (y_pos & 0x07);
    if (color) {
      oled_fb[x + y_page_offset] |= bit_to_set;
    }
    else {
      bit_to_set = ~bit_to_set;
      oled_fb[x + y_page_offset] &= bit_to_set;
    }
  }
}

/*
 * Draw a rectangle on the display.
 * I guess just pick the longer dimension, and either draw
 * horizontal or vertical lines.
 * Notable args:
 *   - outline: If <=0, fill the rectangle with 'color'.
 *        If >0, draw an outline inside the dimensions of N pixels.
 *   - color: If 0, clear drawn bits. If not 0, set drawn bits.
 */
inline void oled_draw_rect(int x, int y, int w, int h,
                           int outline, unsigned char color) {
  if (outline > 0) {
    // Draw an outline.
    int o_pos;
    // Top.
    for (o_pos = y; o_pos < (y+outline); ++o_pos) {
      oled_draw_h_line(x, o_pos, w, color);
    }
    // Bottom.
    for (o_pos = (y+h-1); o_pos > (y+h-1-outline); --o_pos) {
      oled_draw_h_line(x, o_pos, w, color);
    }
    // Left.
    for (o_pos = x; o_pos < (x+outline); ++o_pos) {
      oled_draw_v_line(o_pos, y, h, color);
    }
    // Right.
    for (o_pos = (x+w-1); o_pos > (x+w-1-outline); --o_pos) {
      oled_draw_v_line(o_pos, y, h, color);
    }
  }
  else {
    // Draw a filled rectangle.
    if (w > h) {
      // Draw fewer horizontal lines than vertical ones.
      int y_pos;
      for (y_pos = y; y_pos < (y+h); ++y_pos) {
        oled_draw_h_line(x, y_pos, w, color);
      }
    }
    else {
      // Draw fewer (or ==) vertical lines than horizontal ones.
      int x_pos;
      for (x_pos = x; x_pos < (x+w); ++x_pos) {
        oled_draw_v_line(x_pos, y, h, color);
      }
    }
  }
}

/*
 * Write a pixel in the current OLED framebuffer.
 * Note that the positioning is a bit odd; each byte is a VERTICAL column
 * of 8 pixels, but each successive byte increments the row position by 1.
 * This means that the buffer is 8x 128-byte pages stacked on top of one
 * another. To set an (x, y) pixel, we |= one position in one byte.
 *   Byte offset = x + ((y / 8) * 128)
 *   Bit offset  = (y & 0x07)
 * 'color' indicates whether to set or unset the pixel. 0 means 'unset.'
 */
inline void oled_write_pixel(int x, int y, unsigned char color) {
  int y_page = y / 8;
  int byte_to_mod = x + (y_page * 128);
  int bit_to_set = 0x01 << (y & 0x07);
  if (color) {
    oled_fb[byte_to_mod] |= bit_to_set;
  }
  else {
    bit_to_set = ~bit_to_set;
    oled_fb[byte_to_mod] &= bit_to_set;
  }
}

void oled_draw_letter(int x, int y, unsigned int w0, unsigned int w1, unsigned char color, char size) {
  // TODO: Make this more efficient than drawing
  // pixels one-by-one.
  int w_iter = 0;
  int cur_x = x;
  int cur_y = y;
  unsigned int aw0 = w0;
  unsigned int aw1 = w1;
  if (!color) {
    aw0 = ~aw0;
    aw1 = ~aw1;
  }
  int px_incr = 1;
  int line_h = 8;
  unsigned char t_col = 0x00;
  int cx = cur_x;
  int cy = cur_y;
  if (size == 'L') {
    px_incr = 2;
    line_h = 16;
  }
  for (w_iter = 31; w_iter >= 0; --w_iter) {
    t_col = !(!(aw0 & (1 << w_iter)));
    for (cx = cur_x; cx < cur_x + px_incr; ++cx) {
      for (cy = cur_y; cy < cur_y + px_incr; ++cy) {
        oled_write_pixel(cx, cy, t_col);
      }
    }
    cur_y += px_incr;
    if (cur_y == y+line_h) {
      cur_y = y;
      cur_x += px_incr;
    }
  }
  for (w_iter = 15; w_iter >= 0; --w_iter) {
    t_col = !(!(aw1 & (1 << w_iter)));
    for (cx = cur_x; cx < cur_x + px_incr; ++cx) {
      for (cy = cur_y; cy < cur_y + px_incr; ++cy) {
        oled_write_pixel(cx, cy, t_col);
      }
    }
    cur_y += px_incr;
    if (cur_y == y+line_h) {
      cur_y = y;
      cur_x += px_incr;
    }
  }
}

void oled_draw_letter_c(int x, int y, char c, unsigned char color, char size) {
  unsigned int w0 = 0x00;
  unsigned int w1 = 0x00;
  if (c == 'A') {
    w0 = OLED_CH_A0;
    w1 = OLED_CH_A1B1 >> 16;
  }
  else if (c == 'B') {
    w0 = OLED_CH_B0;
    w1 = OLED_CH_A1B1 & 0x0000FFFF;
  }
  else if (c == 'C') {
    w0 = OLED_CH_C0;
    w1 = OLED_CH_C1D1 >> 16;
  }
  else if (c == 'D') {
    w0 = OLED_CH_D0;
    w1 = OLED_CH_C1D1 & 0x0000FFFF;
  }
  else if (c == 'E') {
    w0 = OLED_CH_E0;
    w1 = OLED_CH_E1F1 >> 16;
  }
  else if (c == 'F') {
    w0 = OLED_CH_F0;
    w1 = OLED_CH_E1F1 & 0x0000FFFF;
  }
  else if (c == 'G') {
    w0 = OLED_CH_G0;
    w1 = OLED_CH_G1H1 >> 16;
  }
  else if (c == 'H') {
    w0 = OLED_CH_H0;
    w1 = OLED_CH_G1H1 & 0x0000FFFF;
  }
  else if (c == 'I') {
    w0 = OLED_CH_I0;
    w1 = OLED_CH_I1J1 >> 16;
  }
  else if (c == 'J') {
    w0 = OLED_CH_J0;
    w1 = OLED_CH_I1J1 & 0x0000FFFF;
  }
  else if (c == 'K') {
    w0 = OLED_CH_K0;
    w1 = OLED_CH_K1L1 >> 16;
  }
  else if (c == 'L') {
    w0 = OLED_CH_L0;
    w1 = OLED_CH_K1L1 & 0x0000FFFF;
  }
  else if (c == 'M') {
    w0 = OLED_CH_M0;
    w1 = OLED_CH_M1N1 >> 16;
  }
  else if (c == 'N') {
    w0 = OLED_CH_N0;
    w1 = OLED_CH_M1N1 & 0x0000FFFF;
  }
  else if (c == 'O') {
    w0 = OLED_CH_O0;
    w1 = OLED_CH_O1P1 >> 16;
  }
  else if (c == 'P') {
    w0 = OLED_CH_P0;
    w1 = OLED_CH_O1P1 & 0x0000FFFF;
  }
  else if (c == 'Q') {
    w0 = OLED_CH_Q0;
    w1 = OLED_CH_Q1R1 >> 16;
  }
  else if (c == 'R') {
    w0 = OLED_CH_R0;
    w1 = OLED_CH_Q1R1 & 0x0000FFFF;
  }
  else if (c == 'S') {
    w0 = OLED_CH_S0;
    w1 = OLED_CH_S1T1 >> 16;
  }
  else if (c == 'T') {
    w0 = OLED_CH_T0;
    w1 = OLED_CH_S1T1 & 0x0000FFFF;
  }
  else if (c == 'U') {
    w0 = OLED_CH_U0;
    w1 = OLED_CH_U1V1 >> 16;
  }
  else if (c == 'V') {
    w0 = OLED_CH_V0;
    w1 = OLED_CH_U1V1 & 0x0000FFFF;
  }
  else if (c == 'W') {
    w0 = OLED_CH_W0;
    w1 = OLED_CH_W1X1 >> 16;
  }
  else if (c == 'X') {
    w0 = OLED_CH_X0;
    w1 = OLED_CH_W1X1 & 0x0000FFFF;
  }
  else if (c == 'Y') {
    w0 = OLED_CH_Y0;
    w1 = OLED_CH_Y1Z1 >> 16;
  }
  else if (c == 'Z') {
    w0 = OLED_CH_Z0;
    w1 = OLED_CH_Y1Z1 & 0x0000FFFF;
  }
  else if (c == 'a') {
    w0 = OLED_CH_a0;
    w1 = OLED_CH_a1b1 >> 16;
  }
  else if (c == 'b') {
    w0 = OLED_CH_b0;
    w1 = OLED_CH_a1b1 & 0x0000FFFF;
  }
  else if (c == 'c') {
    w0 = OLED_CH_c0;
    w1 = OLED_CH_c1d1 >> 16;
  }
  else if (c == 'd') {
    w0 = OLED_CH_d0;
    w1 = OLED_CH_c1d1 & 0x0000FFFF;
  }
  else if (c == 'e') {
    w0 = OLED_CH_e0;
    w1 = OLED_CH_e1f1 >> 16;
  }
  else if (c == 'f') {
    w0 = OLED_CH_f0;
    w1 = OLED_CH_e1f1 & 0x0000FFFF;
  }
  else if (c == 'g') {
    w0 = OLED_CH_g0;
    w1 = OLED_CH_g1h1 >> 16;
  }
  else if (c == 'h') {
    w0 = OLED_CH_h0;
    w1 = OLED_CH_g1h1 & 0x0000FFFF;
  }
  else if (c == 'i') {
    w0 = OLED_CH_i0;
    w1 = OLED_CH_i1j1 >> 16;
  }
  else if (c == 'j') {
    w0 = OLED_CH_j0;
    w1 = OLED_CH_i1j1 & 0x0000FFFF;
  }
  else if (c == 'k') {
    w0 = OLED_CH_k0;
    w1 = OLED_CH_k1l1 >> 16;
  }
  else if (c == 'l') {
    w0 = OLED_CH_l0;
    w1 = OLED_CH_k1l1 & 0x0000FFFF;
  }
  else if (c == 'm') {
    w0 = OLED_CH_m0;
    w1 = OLED_CH_m1n1 >> 16;
  }
  else if (c == 'n') {
    w0 = OLED_CH_n0;
    w1 = OLED_CH_m1n1 & 0x0000FFFF;
  }
  else if (c == 'o') {
    w0 = OLED_CH_o0;
    w1 = OLED_CH_o1p1 >> 16;
  }
  else if (c == 'p') {
    w0 = OLED_CH_p0;
    w1 = OLED_CH_o1p1 & 0x0000FFFF;
  }
  else if (c == 'q') {
    w0 = OLED_CH_q0;
    w1 = OLED_CH_q1r1 >> 16;
  }
  else if (c == 'r') {
    w0 = OLED_CH_r0;
    w1 = OLED_CH_q1r1 & 0x0000FFFF;
  }
  else if (c == 's') {
    w0 = OLED_CH_s0;
    w1 = OLED_CH_s1t1 >> 16;
  }
  else if (c == 't') {
    w0 = OLED_CH_t0;
    w1 = OLED_CH_s1t1 & 0x0000FFFF;
  }
  else if (c == 'u') {
    w0 = OLED_CH_u0;
    w1 = OLED_CH_u1v1 >> 16;
  }
  else if (c == 'v') {
    w0 = OLED_CH_v0;
    w1 = OLED_CH_u1v1 & 0x0000FFFF;
  }
  else if (c == 'w') {
    w0 = OLED_CH_w0;
    w1 = OLED_CH_w1x1 >> 16;
  }
  else if (c == 'x') {
    w0 = OLED_CH_x0;
    w1 = OLED_CH_w1x1 & 0x0000FFFF;
  }
  else if (c == 'y') {
    w0 = OLED_CH_y0;
    w1 = OLED_CH_y1z1 >> 16;
  }
  else if (c == 'z') {
    w0 = OLED_CH_z0;
    w1 = OLED_CH_y1z1 & 0x0000FFFF;
  }
  else if (c == '0') {
    w0 = OLED_CH_00;
    w1 = OLED_CH_0111 >> 16;
  }
  else if (c == '1') {
    w0 = OLED_CH_10;
    w1 = OLED_CH_0111 & 0x0000FFFF;
  }
  else if (c == '2') {
    w0 = OLED_CH_20;
    w1 = OLED_CH_2131 >> 16;
  }
  else if (c == '3') {
    w0 = OLED_CH_30;
    w1 = OLED_CH_2131 & 0x0000FFFF;
  }
  else if (c == '4') {
    w0 = OLED_CH_40;
    w1 = OLED_CH_4151 >> 16;
  }
  else if (c == '5') {
    w0 = OLED_CH_50;
    w1 = OLED_CH_4151 & 0x0000FFFF;
  }
  else if (c == '6') {
    w0 = OLED_CH_60;
    w1 = OLED_CH_6171 >> 16;
  }
  else if (c == '7') {
    w0 = OLED_CH_70;
    w1 = OLED_CH_6171 & 0x0000FFFF;
  }
  else if (c == '8') {
    w0 = OLED_CH_80;
    w1 = OLED_CH_8191 >> 16;
  }
  else if (c == '9') {
    w0 = OLED_CH_90;
    w1 = OLED_CH_8191 & 0x0000FFFF;
  }
  else if (c == ':') {
    w0 = OLED_CH_col0;
    w1 = OLED_CH_col1per1 >> 16;
  }
  else if (c == '.') {
    w0 = OLED_CH_per0;
    w1 = OLED_CH_col1per1 & 0x0000FFFF;
  }
  else if (c == '!') {
    w0 = OLED_CH_exc0;
    w1 = OLED_CH_exc1fws1 >> 16;
  }
  else if (c == '/') {
    w0 = OLED_CH_fws0;
    w1 = OLED_CH_exc1fws1 & 0x0000FFFF;
  }
  else if (c == '-') {
    w0 = OLED_CH_hyp0;
    w1 = OLED_CH_hyp1pls1 >> 16;
  }
  else if (c == '+') {
    w0 = OLED_CH_pls0;
    w1 = OLED_CH_hyp1pls1 & 0x0000FFFF;
  }
  else if (c == '<') {
    w0 = OLED_CH_lct0;
    w1 = OLED_CH_lct1rct1 >> 16;
  }
  else if (c == '>') {
    w0 = OLED_CH_rct0;
    w1 = OLED_CH_lct1rct1 & 0x0000FFFF;
  }
  oled_draw_letter(x, y, w0, w1, color, size);
}

void oled_draw_letter_i(int x, int y, int ic, unsigned char color, char size) {
  int magnitude = 1000000000;
  int cur_x = x;
  int first_found = 0;
  int proc_val = ic;
  if (proc_val < 0) {
    proc_val = proc_val * -1;
    oled_draw_letter_c(cur_x, y, '-', color, size);
    if (size == 'S') {
      cur_x += 6;
    }
    else if (size == 'L') {
      cur_x += 12;
    }
  }
  for (magnitude = 1000000000; magnitude > 0; magnitude = magnitude / 10) {
    int m_val = proc_val / magnitude;
    proc_val -= (m_val * magnitude);
    if (m_val > 0 || first_found || magnitude == 1) {
      first_found = 1;
      char mc = ' ';
      if (m_val == 0) {
        mc = '0';
      }
      else if (m_val == 1) {
        mc = '1';
      }
      else if (m_val == 2) {
        mc = '2';
      }
      else if (m_val == 3) {
        mc = '3';
      }
      else if (m_val == 4) {
        mc = '4';
      }
      else if (m_val == 5) {
        mc = '5';
      }
      else if (m_val == 6) {
        mc = '6';
      }
      else if (m_val == 7) {
        mc = '7';
      }
      else if (m_val == 8) {
        mc = '8';
      }
      else if (m_val == 9) {
        mc = '9';
      }
      oled_draw_letter_c(cur_x, y, mc, color, size);
      if (size == 'S') {
        cur_x += 6;
      }
      else if (size == 'L') {
        cur_x += 12;
      }
      if (cur_x >= 128) { return; }
    }
  }
}

void oled_draw_text(int x, int y, char* cc, unsigned char color, char size) {
  int i = 0;
  int offset = 0;
  while (cc[i] != '\0') {
    oled_draw_letter_c(x + offset, y, cc[i], color, size);
    if (size == 'S') {
      offset += 6;
    }
    else if (size == 'L') {
      offset += 12;
    }
    ++i;
  }
}

void draw_main_menu(void) {
  oled_draw_rect(0, 0, 128, 64, 0, 0);
  // Only use the middle 96 pixels, to make this easier
  // to transition to a color display.
  oled_draw_rect(15, 0, 96, 64, 2, 1);
  // Draw a big 'TETRIS' in the top-middle.
  oled_draw_text(27, 12, "TETRIS\0", 1, 'L');
  // Draw menu options.
  // (Currently just 'Start')
  oled_draw_text(50, 40, "Start\0", 1, 'S');
  // Draw a little triangle next to it.
  oled_draw_v_line(40, 42, 5, 1);
  oled_draw_v_line(41, 43, 3, 1);
  oled_write_pixel(42, 44, 1);
}

void draw_game_over(void) {
  oled_draw_rect(0, 0, 128, 64, 0, 0);
  // Only use the middle 96 pixels, to make this easier
  // to transition to a color display.
  oled_draw_rect(15, 0, 96, 64, 2, 1);
  // Draw a bit 'GAME OVER' label.
  oled_draw_text(39, 12, "GAME\0", 1, 'L');
  oled_draw_text(39, 36, "OVER\0", 1, 'L');
}

void draw_tetris_game(void) {
  oled_draw_rect(0, 0, 128, 64, 0, 0);
  // Only use the middle 96 pixels, to make this easier
  // to transition to a color display.
  oled_draw_rect(15, 0, 96, 64, 2, 1);
  // Draw a test grid, 10x20 @3 square pixels.
  uint8_t grid_ix = 0;
  uint8_t grid_iy = 0;
  // Vertical 'column' lines.
  for (grid_ix = 0; grid_ix < 11; ++grid_ix) {
    oled_draw_v_line(47 + (grid_ix * 3), 2, 60, 1);
  }
  // Horizontal 'row' lines.
  for (grid_iy = 0; grid_iy < 21; ++grid_iy) {
    oled_draw_h_line(48, 2 + (grid_iy * 3), 30, 1);
  }

  // Draw the grid.
  for (grid_ix = 0; grid_ix < 10; ++grid_ix) {
    for (grid_iy = 0; grid_iy < 20; ++grid_iy) {
      // For monochrome displays, just check empty/not empty.
      if (tetris_grid[grid_ix][grid_iy] != TGRID_EMPTY) {
        oled_draw_rect(48 + (grid_ix * 3),
                       3 + (grid_iy * 3),
                       2, 2, 0, 1);
      }
    }
  }

  // Draw the current brick.
  for (grid_ix = 0; grid_ix < 4; ++grid_ix) {
    for (grid_iy = 0; grid_iy < 4; ++grid_iy) {
      if ((cur_block_y+grid_iy >= 0) &&
          (BRICKS[cur_block_r][cur_block_type] & (1 << (3-grid_ix+(3-grid_iy)*4)))) {
        oled_draw_rect(48 + ((cur_block_x+grid_ix) * 3),
                       3 + ((cur_block_y+grid_iy) * 3),
                       2, 2, 0, 1);
      }
    }
  }
}

/*
 * 'Reset Game State' method, to start a new game.
 */
void reset_game_state(void) {
  // Reset the 'current block' position.
  cur_block_x = 4;
  cur_block_y = -1;
  cur_block_r = 0;
  // Clear the grid memory.
  uint8_t grid_ix = 0;
  uint8_t grid_iy = 0;
  for (grid_ix = 0; grid_ix < 10; ++grid_ix) {
    for (grid_iy = 0; grid_iy < 20; ++grid_iy) {
      tetris_grid[grid_ix][grid_iy] = TGRID_EMPTY;
    }
  }
}

/*
 * Check whether the current brick can rotate into a given
 * position. Return 1 if there is a collision, 0 if it can rotate.
 */
uint8_t check_brick_rot(int8_t new_r) {
  uint8_t grid_ix = 0;
  uint8_t grid_iy = 0;
  for (grid_ix = 0; grid_ix < 4; ++grid_ix) {
    for (grid_iy = 0; grid_iy < 4; ++grid_iy) {
      if ((cur_block_y+grid_iy >= 0) &&
          (BRICKS[new_r][cur_block_type] & (1 << (3-grid_ix+(3-grid_iy)*4))) &&
          ((cur_block_y+grid_iy > 19) ||
           (cur_block_x+grid_ix < 0) ||
           (cur_block_x+grid_ix > 9) ||
           (tetris_grid[cur_block_x+grid_ix][cur_block_y+grid_iy] != TGRID_EMPTY))) {
        return 1;
      }
    }
  }
  return 0;
}

/*
 * Check whether the current brick can move into a
 * given grid coordinate.
 * Return 1 if there is a collision, 0 if the space is free.
 */
uint8_t check_brick_pos(int8_t xp, int8_t yp) {
  uint8_t grid_ix = 0;
  uint8_t grid_iy = 0;
  for (grid_ix = 0; grid_ix < 4; ++grid_ix) {
    for (grid_iy = 0; grid_iy < 4; ++grid_iy) {
      if ((yp+grid_iy >= 0) &&
          (BRICKS[cur_block_r][cur_block_type] & (1 << (3-grid_ix+(3-grid_iy)*4))) &&
          ((yp+grid_iy > 19) || (xp+grid_ix < 0) ||
           (xp+grid_ix > 9) ||
           (tetris_grid[xp+grid_ix][yp+grid_iy] != TGRID_EMPTY))) {
        return 1;
      }
    }
  }
  return 0;
}

/*
 * Main 'tick' for the Tetris game loop.
 * This performs one 'step' in the game, either dropping a brick
 * or setting it in place and clearing rows/creating the next one.
 */
void tetris_game_tick(void) {
  uint8_t grid_ix = 0;
  uint8_t grid_iy = 0;
  unsigned char can_drop = 1;
  /* Step 1:  Try to drop the current brick by 1 cell. */
  if (check_brick_pos(cur_block_x, cur_block_y+1)) {
    can_drop = 0;
  }

  if (can_drop) {
    /* Step 2a: If the current brick can drop, do so. */
    cur_block_y++;
  }
  else {
    /* Step 2b: If the current brick cannot drop, fix it
     *          in the main Tetris grid. */
    for (grid_ix = 0; grid_ix < 4; ++grid_ix) {
      for (grid_iy = 0; grid_iy < 4; ++grid_iy) {
        if (BRICKS[cur_block_r][cur_block_type] & (1 << (3-grid_ix+(3-grid_iy)*4))) {
          if (cur_block_y+grid_iy < 0) {
            // Game over
            game_state = GAME_STATE_GAME_OVER;
            uled_state = 0;
            stop_timer(TIM2);
          }
          else {
            tetris_grid[cur_block_x+grid_ix][cur_block_y+grid_iy] = cur_block_type;
          }
        }
      }
    }
    /* Step 3b: Clear any appropriate rows. */
    /* Step 4b: Create a new 'current brick'. */
    cur_block_x = 4;
    cur_block_y = -1;
    cur_block_r = 0;
  }
}
