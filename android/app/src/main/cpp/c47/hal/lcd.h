// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file hal/lcd.h
 * LCD related functions.
 * DMCP functions should follow specification at 
 * https://technical.swissmicros.com/dmcp/doc/DMCP-ifc-html/group__lcd__graphics.html
 */
#if defined(PC_BUILD) && !defined(WIN32)
void print_caller(const char *format, ...);
#else
static inline void print_caller(const char *format, ...) {}
#endif //PC_BUILD

#if !defined(LCD_H)
  #define LCD_H

#if defined(DMCP_BUILD)
  static inline void _lcdRefresh(void) {
    lcd_forced_refresh();
  }
  static inline void _lcdSBRefresh(void) {
    lcd_refresh_lines(0,20);
  }
  static inline void _lcdBandRefresh(uint32_t y, uint32_t dy) {
    lcd_refresh_lines(y,dy);
  }
  // lcd_fill_rect from dmcp.h
  // lcd_refresh   from dmcp.h
  // LCD_write_line from dmcp.h

#else
  #define 	LCD_LINE_SIZE   50
  #define 	LCD_LINE_BUF_SIZE   (LCD_LINE_SIZE+4)
  #define LCD_SET_VALUE   0   // Black pixel
  #define LCD_EMPTY_VALUE 255 // White (or empty) pixel
  typedef enum { BLT_OR = 0, BLT_ANDN = 1, BLT_XOR = 2 } blt_op_t;
  typedef enum { BLT_NONE = 0, BLT_SET  = 1 } blt_fill_t;
  extern gboolean ui_is_active;

  
  // void lcd_fill_ptrn(int x, int y, int dx, int dy, int ptrn1, int ptrn2);
  /**
   * Refresh the LCD with the draw buffer contents.
   */
  void _lcdRefresh    (void);
  void _lcdSBRefresh  (void);
  void _lcdBandRefresh(uint32_t y, uint32_t dy);

  // mimic DMCP functions
 /**
  * Fills a rectangle with either black or white.
  *
  * \param[in] x   x coordinate from 0 (left) to 399 (right)
  * \param[in] y   y coordinate from 0 (top) to 239 (bottom)
  * \param[in] dx  width of the rect (inclusive of the ends)
  * \param[in] dy  height of the rect (inclusive of the ends)
  * \param[in] val LCD_SET_VALUE (black) or LCD_EMPTY_VALUE (white) for the filled
  *                rectangle
  */
  void lcd_fill_rect (uint32_t x, uint32_t y, uint32_t dx, uint32_t dy, int val);
  
  // send a line buffer directly to LCD bypassing lcd_buffer
  // buf has length LCD_LINE_BUF_SIZE (54byte)
  // buf[0] = 0 means line is unchanged, 1 if line is changed (dirty)
  // buf[1] = row number where 0 is bottom line
  // Update the gtk screen only on this line
  void LCD_write_line (uint8_t *buf);
  
  // get address to first data byte for the line
  // i.e. lcd_line_addr(0) is (lcd_buffer + )
  // also marks the line dirty
  uint8_t * lcd_line_addr ( int y);
  
  /**
  * Blits dx bits from val at position x, y.
  *
  * \param[in] x       Position x
  * \param[in] dx      (1-24) Width x
  * \param[in] y       Position y
  * \param[in] val     Value to blit
  * \param[in] blt_op  Blit operation BLT_OR, BLT_ANDN or BLT_XOR
  * \param[in] fill    BLT_SET or BLT_NONE
  *
  * Value of fill doesn't apply for BLT_XOR.
  *
  * BLT_NONE doesn't affect any blit operation.
  * BLT_SET affects src value of blit operation:
  *
  * for BLT_OR src = 0 over width of operation (i.e. dx)
  * for BLT_ANDN src = 1 over width of operation (i.e. dx)
  */
  void bitblt24 ( uint32_t x, uint32_t dx, uint32_t y, uint32_t val, int blt_op, int fill );
  
  // set buffer lcd data to 0 and set row numbers
  // no sure the original sets the row numbers
  void lcd_clear_buf ();
  
  // update LCD lines according to data in lcd_buffer
  // skips line if not marked dirty
  void lcd_refresh ();
  
  // write cnt lines  starting with line ln regardless update mark
  void lcd_refresh_lines (uint8_t ln, uint8_t cnt);

  void refresh_gui(void);
#endif // DMCP_BUILD
#if !defined(TESTSUITE_BUILD)
 /**
  * Sets a single black pixel on the screen.
  *
  * \param[in] x x coordinate from 0 (left) to 399 (right)
  * \param[in] y y coordinate from 0 (top) to 239 (bottom)
  */
  static inline void setBlackPixel(uint32_t x, uint32_t y) {
    bitblt24(x, 1, y, 1, BLT_OR,   BLT_NONE);
  }

 /**
  * Sets a single white pixel on the screen.
  *
  * \param[in] x x coordinate from 0 (left) to 399 (right)
  * \param[in] y y coordinate from 0 (top) to 239 (bottom)
  */
  static inline void setWhitePixel(uint32_t x, uint32_t y) {
    bitblt24(x, 1, y, 1, BLT_ANDN, BLT_NONE);
  }

 /**
  * Turns a single black pixel to a white pixel or vice versa on the screen.
  *
  * \param[in] x x coordinate from 0 (left) to 399 (right)
  * \param[in] y y coordinate from 0 (top) to 239 (bottom)
  */
  static inline void flipPixel(uint32_t x, uint32_t y) {
    bitblt24(x, 1, y, 1, BLT_XOR,  BLT_NONE);
  }
#endif // !TESTSUITE_BUILD
#endif // !LCD_H
