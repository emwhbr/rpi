// ************************************************************************
// *                                                                      *
// * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __LCD6100_IO_H__
#define __LCD6100_IO_H__

#include "lcd6100.h"
#include "lcd6100_gpio.h"
#include "lcd6100_bmp.h"
#include "lcd6100_font_small.h"
#include "lcd6100_font_medium.h"
#include "lcd6100_font_large.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_io {

public:
  lcd6100_io(uint8_t hw_reset_pin);
  ~lcd6100_io(void);

  void initialize(void);

  void finalize(void);

  void fill_screen(LCD6100_COLOUR colour);

  void draw_pixel(uint8_t row,
		  uint8_t col,
		  LCD6100_COLOUR colour);

  void draw_line(uint8_t start_row,
		 uint8_t start_col,
		 uint8_t end_row,
		 uint8_t end_col,
		 LCD6100_COLOUR colour);

  void draw_rectangle(uint8_t start_row,
		      uint8_t start_col,
		      uint8_t end_row,
		      uint8_t end_col,
		      bool filled,
		      LCD6100_COLOUR colour);

  void draw_circle(uint8_t row,
		   uint8_t col,
		   uint8_t radius,
		   LCD6100_COLOUR colour);

  void draw_bmp_image(uint8_t row,
		      uint8_t col,
		      string bmp_image,
		      bool scale);

  void write_char(char c,
		  uint8_t row,
		  uint8_t col,		  
		  LCD6100_COLOUR fg_colour,
		  LCD6100_COLOUR bg_colour,
		  LCD6100_FONT font);

  void write_string(const char *str,
		    uint8_t row,
		    uint8_t col,		  
		    LCD6100_COLOUR fg_colour,
		    LCD6100_COLOUR bg_colour,
		    LCD6100_FONT font);
  
  void write_command(uint8_t cmd);

  void write_data(uint8_t data);

protected:
  virtual void spi_initialize(void) =0;
  virtual void spi_finalize(void) =0;
  virtual void spi_write(const uint16_t *msg) =0;

private:
  // GPIO
  uint8_t                m_hw_reset_pin;
  LCD6100_GPIO_FUNCTION  m_hw_reset_pin_func;
  uint8_t                m_hw_reset_pin_val;
  lcd6100_gpio           m_gpio;

  // Font tables
  lcd6100_font_small  m_font_small;
  lcd6100_font_medium m_font_medium;
  lcd6100_font_large  m_font_large;

  void init_members(void);

  void init_lcd_controller(void);

  void finalize_lcd_controller(void);

  void set_drawing_limits(uint8_t start_row,
			  uint8_t start_col,
			  uint8_t end_row,
			  uint8_t end_col);

  void draw_filled_rectangle(uint8_t start_row,
			     uint8_t start_col,
			     uint8_t end_row,
			     uint8_t end_col,
			     LCD6100_COLOUR colour);

  void draw_empty_rectangle(uint8_t start_row,
			    uint8_t start_col,
			    uint8_t end_row,
			    uint8_t end_col,
			    LCD6100_COLOUR colour);

  lcd6100_font* get_font(LCD6100_FONT font);
};

#endif // __LCD6100_IO_H__
