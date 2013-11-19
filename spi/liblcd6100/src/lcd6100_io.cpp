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

#include <string.h>

#include "lcd6100_io.h"
#include "lcd6100_exception.h"
#include "lcd6100_delay.h"
#include "lcd6100_hw.h"

// Implementation notes:
// 1. Assumes Philips LCD controller PCF8833.
//
// 2. LCD6610 has a 132 x 132 pixel matrix.
//    Coordinate system has its origo in the bottom left corner:
//    (row, col)th pixel is
//      col pixels from the left
//      row pixels from the bottom
//                              ____
//                              |  |
//          ___________________|____|_____
//          |                            |
//          |-----------------------------
//    ^     |                            | (Row=131, Col=131)
//    |     |                            |
//    |     |                            |
//    |     |                            |
//    |     |                            |
//    |     |                            |
//    |     |                            |
//   ROW    |                            |
//    |     |                            |                            |
//    |     |                            |
//    |     |                            |
//    |     |                            |
//    |     |____________________________| (Row=0, Col=131)
//    |   origo
//    |----------------- COL ----------------------->
//
// 3. Each pixel has 12-bit RGB, 4096 colours (rrrrggggbbbb).
//
// 4. We make use of PCF8833 "wrap around" feature.
//    By defining a drawing box, the memory can be filled by
//    successive memory writes until all pixels have been written.
//
// 5. Line and circle drawing algorithms devloped by Jack Elton Bresenham (1962).
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_io::lcd6100_io(uint8_t hw_reset_pin)
{
  m_hw_reset_pin = hw_reset_pin;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

lcd6100_io::~lcd6100_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::initialize(void)
{
  spi_initialize();      // Initialize SPI layer
  init_lcd_controller(); // Initialize LCD controller
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::finalize(void)
{
  finalize_lcd_controller(); // Finalize LCD controller
  spi_finalize();            // Finalize SPI layer

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::fill_screen(LCD6100_COLOUR colour)
{
  // Drawing area is entire screen
  set_drawing_limits(0, 0,
		     LCD6100_ROW_MAX_ADDR, LCD6100_COL_MAX_ADDR);

  const unsigned tot_nr_pixels = 
    (LCD6100_ROW_MAX_ADDR + 1) * (LCD6100_COL_MAX_ADDR + 1);

  // Fill screen using specified RGB colour value
  // Two pixels = 2 * 12bit = 24bit = 3 bytes
  write_command(CMD_RAMWR);
  for (unsigned i=0; i < (tot_nr_pixels / 2); i++) {
    write_data( (colour.wd >> 4) & 0xFF );
    write_data( ((colour.wd & 0x0F) << 4) | ((colour.wd >> 8) & 0x0F) );
    write_data( colour.wd & 0xFF );
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_pixel(uint8_t row,
			    uint8_t col,
			    LCD6100_COLOUR colour)
{
  // Drawing area is one pixel
  set_drawing_limits(row, col,
		     row, col);

  // Write pixel
  // Two pixels = 2 * 12bit = 24bit = 3 bytes
  // Last pixel will be terminated by NOP
  write_command(CMD_RAMWR);
  write_data( (colour.wd >> 4) & 0xFF );
  write_data( ((colour.wd & 0x0F) << 4) | 0x00 );
  write_command(CMD_NOP);
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_line(uint8_t start_row,
			   uint8_t start_col,
			   uint8_t end_row,
			   uint8_t end_col,
			   LCD6100_COLOUR colour)
{
  ////////////////////////////////////////////
  // BRESENHAAM ALGORITHM FOR LINE DRAWING
  ///////////////////////////////////////////

  int dx, dy;
  int sdx, sdy;
  int dxabs, dyabs;
  int x, y;
  int px, py;

  dx = end_row - start_row;  // Horizontal distance of the line
  dy = end_col - start_col;  // Vertical distance of the line
  dxabs = abs(dx);
  dyabs = abs(dy);
  sdx = ( (dx < 0) ? -1 : 1 );
  sdy = ( (dy < 0) ? -1 : 1 );
  x = dyabs >> 1;
  y = dxabs >> 1;
  px = start_row;
  py = start_col;

  draw_pixel(px, py, colour);

  if (dxabs >= dyabs) {

    // Line is more horizontal than vertical

    for (int i=0; i < dxabs; i++) {
      y += dyabs;
      if (y >= dxabs) {
        y -= dxabs;
        py += sdy;
      }
      px += sdx;
      draw_pixel(px, py, colour);
    }
  }
  else {

    // Line is more vertical than horizontal

    for (int i=0; i < dyabs; i++) {
      x += dxabs;
      if (x >= dyabs) {
        x -= dyabs;
        px += sdx;
      }
      py += sdy;
      draw_pixel(px, py, colour);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_rectangle(uint8_t start_row,
				uint8_t start_col,
				uint8_t end_row,
				uint8_t end_col,
				bool filled,
				LCD6100_COLOUR colour)
{
  if (filled) {
    draw_filled_rectangle(start_row,
			  start_col,
			  end_row,
			  end_col,
			  colour);
  }
  else {
    draw_empty_rectangle(start_row,
			 start_col,
			 end_row,
			 end_col,
			 colour);
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_circle(uint8_t row,
			     uint8_t col,
			     uint8_t radius,
			     LCD6100_COLOUR colour)
{
  ////////////////////////////////////////////
  // BRESENHAAM ALGORITHM FOR CIRCLE DRAWING
  ///////////////////////////////////////////

  int x = 0;
  int y = radius;
  int d = (3 - 2*radius);
  
  while (x < y) {
    
    x++;

    if (d < 0) {
      d += (4*x + 6);
    }
    else {
      y--;
      d += 4*(x-y) + 10;
    }
    
    draw_pixel(row + x, col + y, colour);
    draw_pixel(row - x, col + y, colour);
    draw_pixel(row + x, col - y, colour);
    draw_pixel(row - x, col - y, colour);
    draw_pixel(row + y, col + x, colour);
    draw_pixel(row - y, col + x, colour);
    draw_pixel(row + y, col - x, colour);
    draw_pixel(row - y, col - x, colour);
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_bmp_image(uint8_t row,
				uint8_t col,
				string bmp_image,
				bool scale)
{
  lcd6100_bmp *bmp = new lcd6100_bmp(bmp_image);

  try {
    // Parse image
    bmp->parse(scale);

    // Check that image will fit on screen
    // Assumes start row and col already checked by caller
    uint8_t end_row = row + bmp->get_height() - 1;
    uint8_t end_col = col + bmp->get_width() - 1;
    
    if ( (end_row > LCD6100_ROW_MAX_ADDR) ||
	 (end_col > LCD6100_COL_MAX_ADDR) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"End row(%u), max(%u). End col(%u), max(%u)",
		end_row, LCD6100_ROW_MAX_ADDR,
		end_col, LCD6100_COL_MAX_ADDR);
    }

    // Drawing area is entire image
    set_drawing_limits(row, col,
		       end_row, end_col);

    // BMP image pixel row
    unsigned bmp_j = bmp->get_height();

    // Work through all pixels, row by row, from the bottom and up
    write_command(CMD_RAMWR);
    for (unsigned bmp_row=0; bmp_row < bmp->get_height(); bmp_row++) {

      bmp_j--; // Decrement BMP image row

      // Work on each pixel in the row (left to right)
      // Note! We do two pixels each loop
      for (unsigned bmp_col=0; bmp_col < bmp->get_width(); bmp_col+=2) {
      
	LCD6100_COLOUR colour_0;  // First pixel
	LCD6100_COLOUR colour_1;  // Second pixel

	// Get first pixel
	colour_0 = bmp->get_pixel(bmp_col, bmp_j);

	// Check if second pixel is on current row
	if ( (bmp_col + 1) < bmp->get_width() ) {

	  // Get second pixel
	  colour_1 = bmp->get_pixel(bmp_col + 1, bmp_j);

	  // Fill pixels using specified RGB colour value
	  // Two pixels = 2 * 12bit = 24bit = 3 bytes
	  write_data( (colour_0.wd >> 4) & 0xFF );
	  write_data( ((colour_0.wd & 0x0F) << 4) | ((colour_1.wd >> 8) & 0x0F) );
	  write_data( colour_1.wd & 0xFF );

	}
	else {
	  // Second pixel is NOT on current row
	  // Write one pixel only
	  // Two pixels = 2 * 12bit = 24bit = 3 bytes
	  // Last pixel will be terminated by NOP
	  write_data( (colour_0.wd >> 4) & 0xFF );
	  write_data( ((colour_0.wd & 0x0F) << 4) | 0x00 );
	  write_command(CMD_NOP);
	}
      }
    }

    delete bmp; 
  }
  catch (...) {
    delete bmp;
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::write_char(char c,
			    uint8_t row,
			    uint8_t col,			   
			    LCD6100_COLOUR fg_colour,
			    LCD6100_COLOUR bg_colour,
			    LCD6100_FONT font)
{
  lcd6100_font *lcd_font = 0;

  // Get specified font object
  lcd_font = get_font(font);

  // Check that character is defined in font table
  uint8_t *font_table = (uint8_t *)lcd_font->get_font_table(c);
  if (!font_table) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "Character('%c') not defined for font(%u)",
	      c, font);
  }
  
  // Check that character will fit on screen
  // Assumes start row and col already checked by caller
  uint8_t end_row = row + lcd_font->get_height() - 1;
  uint8_t end_col = col + lcd_font->get_width() - 1;

  if ( (end_row > LCD6100_ROW_MAX_ADDR) ||
       (end_col > LCD6100_COL_MAX_ADDR) ) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "End row(%u), max(%u). End col(%u), max(%u)",
	      end_row, LCD6100_ROW_MAX_ADDR,
	      end_col, LCD6100_COL_MAX_ADDR);
  }

  // Drawing area is entire character
  set_drawing_limits(row, col,
		     end_row, end_col);

  // Point to last font data byte in character (last row of pixels)
  uint8_t *char_data = font_table + lcd_font->get_bytes_per_char() - 1;

  // Work through all pixels, row by row, from the bottom and up
  write_command(CMD_RAMWR);
  for (int font_row=0; font_row < lcd_font->get_height(); font_row++) {

    // Get pixel row from font table and then decrement row
    uint8_t pixel_row = *char_data--;

    uint8_t pixel_mask = 0x80;
    
    // Work on each pixel in the row (left to right)
    // Note! We do two pixels each loop
    for (int font_col=0; font_col < lcd_font->get_width(); font_col+=2) {
       
       uint16_t pixel_0_wd;
       uint16_t pixel_1_wd;
      
      // Get RGB coulor value for each pixel
      if (pixel_row & pixel_mask) {
	pixel_0_wd = fg_colour.wd;  // Pixel is on
      }
      else {
	pixel_0_wd = bg_colour.wd;  // Pixel is off
      }      
      pixel_mask = pixel_mask >> 1;

      if (pixel_row & pixel_mask) {
	pixel_1_wd = fg_colour.wd;  // Pixel is on
      }
      else {
	pixel_1_wd = bg_colour.wd;  // Pixel is off
      }      
      pixel_mask = pixel_mask >> 1;

      // Fill pixels using specified RGB colour value
      // Two pixels = 2 * 12bit = 24bit = 3 bytes
      write_data( (pixel_0_wd >> 4) & 0xFF );
      write_data( ((pixel_0_wd & 0x0F) << 4) | ((pixel_1_wd >> 8) & 0x0F) );
      write_data( pixel_1_wd & 0xFF );
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::write_string(const char *str,
			      uint8_t row,
			      uint8_t col,			   
			      LCD6100_COLOUR fg_colour,
			      LCD6100_COLOUR bg_colour,
			      LCD6100_FONT font)
{
  lcd6100_font *lcd_font = 0;

  // Get specified font object
  lcd_font = get_font(font);

  // Loop through all characters in string
  for (unsigned i=0; i < strlen(str); i++) {

    uint8_t the_col;

    // Update column to write to
    the_col = col + (i * lcd_font->get_width());

    // Write one character
    write_char(str[i],
	       row,
	       the_col,
	       fg_colour,
	       bg_colour,
	       font);
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::write_command(uint8_t cmd)
{
  uint16_t lcd_cmd = cmd; // Bit 8 is cleared ==> command

  spi_write(&lcd_cmd);    // Send command to LCD
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::write_data(uint8_t data)
{
  uint16_t lcd_data = data | 0x0100; // Bit 8 is set ==> data

  spi_write(&lcd_data);              // Send command to LCD
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::init_members(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::init_lcd_controller(void)
{
  /////////////////////////////
  // Prepare GPIO for HW reset
  /////////////////////////////

  m_gpio.initialize();
  
  // Save old pin function
  m_gpio.get_function(m_hw_reset_pin,
		      m_hw_reset_pin_func);

  // Save old pin value
  m_gpio.read(m_hw_reset_pin,
	      m_hw_reset_pin_val);

  // Set pin as output
  m_gpio.set_function(m_hw_reset_pin,
		      LCD6100_GPIO_FUNC_OUT);

  ////////////////////////////////////////////////
  // Philips PCF8833 LCD controller init sequence
  ////////////////////////////////////////////////

  // Step 1. Hardware reset
  m_gpio.write(m_hw_reset_pin, 1);
  delay(0.001);
  m_gpio.write(m_hw_reset_pin, 0);
  delay(0.001);
  m_gpio.write(m_hw_reset_pin, 1);
  delay(0.001);

  // Step 2. Sleep out
  write_command(CMD_SLEEPOUT);

  // Step 3. Inversion off
  write_command(CMD_INVOFF);

  // Step 4. Colour interface pixel format
  write_command(CMD_COLMOD);
  write_data(0x03);  // 12-bit per pixel

  // Step 5. Memory data access control
  write_command(CMD_MADCTL);
  write_data(0x80);  // Mirror Y, RGB

  // Step 6. Contrast
  write_command(CMD_SETCON);
  write_data(0x30);

  // Step 7. Wait
  delay(0.05);

  // Step 8. Turn ON display
  write_command(CMD_DISPON);
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::finalize_lcd_controller(void)
{
  // Shutdown PCF8833 LCD controller
  write_command(CMD_DISPOFF);
  write_command(CMD_SLEEPIN);

  // Restore GPIO for HW reset
  m_gpio.write(m_hw_reset_pin,
	       m_hw_reset_pin_val);

  m_gpio.set_function(m_hw_reset_pin,
		      m_hw_reset_pin_func);

  m_gpio.finalize();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::set_drawing_limits(uint8_t start_row,
				    uint8_t start_col,
				    uint8_t end_row,
				    uint8_t end_col)
{
  // Row address
  write_command(CMD_PASET);
  write_data(start_row); // Start row
  write_data(end_row);   // End row

  // Column address
  write_command(CMD_CASET);
  write_data(start_col); // Start column
  write_data(end_col);   // End column
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_filled_rectangle(uint8_t start_row,
				       uint8_t start_col,
				       uint8_t end_row,
				       uint8_t end_col,
				       LCD6100_COLOUR colour)
{
  // Determine max- and min values for drawing area
  uint8_t max_row = ( (start_row > end_row) ? start_row : end_row );
  uint8_t max_col = ( (start_col > end_col) ? start_col : end_col );
  uint8_t min_row = ( (start_row <= end_row) ? start_row : end_row );
  uint8_t min_col = ( (start_col <= end_col) ? start_col : end_col );

  // Drawing area is entire rectangle
  set_drawing_limits(min_row, min_col,
		     max_row, max_col);

  // Calculate number of pixels in rectangle
  unsigned nr_pixels = (max_row - min_row + 1) * (max_col - min_col + 1);

  // Fill rectangle using specified RGB colour value
  // Two pixels = 2 * 12bit = 24bit = 3 bytes
  write_command(CMD_RAMWR);
  for (unsigned i=0; i < ( (nr_pixels / 2) + 1 ); i++) {
    write_data( (colour.wd >> 4) & 0xFF );
    write_data( ((colour.wd & 0x0F) << 4) | ((colour.wd >> 8) & 0x0F) );
    write_data( colour.wd & 0xFF );
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::draw_empty_rectangle(uint8_t start_row,
				      uint8_t start_col,
				      uint8_t end_row,
				      uint8_t end_col,
				      LCD6100_COLOUR colour)
{
  draw_line(start_row, start_col, end_row,   start_col, colour);
  draw_line(end_row,   start_col, end_row,   end_col,   colour);
  draw_line(end_row,   end_col,   start_row, end_col,   colour);
  draw_line(start_row, end_col,   start_row, start_col, colour);
}

/////////////////////////////////////////////////////////////////////////////

lcd6100_font* lcd6100_io::get_font(LCD6100_FONT font)
{
  lcd6100_font *lcd_font = 0;

  // Get specified font object
  switch (font) {
  case LCD6100_FONT_SMALL:
    lcd_font = &m_font_small;
    break;
  case LCD6100_FONT_MEDIUM:
    lcd_font = &m_font_medium;
    break;
  case LCD6100_FONT_LARGE:
    lcd_font = &m_font_large;
    break;
  default:
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "Font(%u) not supported", font);
  }

  return lcd_font;
}
