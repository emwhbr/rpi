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

#include <stdlib.h>
#include <string.h>

#include "lcd6100_io.h"
#include "lcd6100_exception.h"
#include "lcd6100_delay.h"
#include "lcd6100_hw.h"

// Implementation notes:
// 1. Assumes Philips LCD controller PCF8833.
//
// 2. Line drawing algorithm devloped by Jack Elton Bresenham (1962).
//
// 3. JOE: Wrap-around feature
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_io::lcd6100_io(LCD6100_CE ce,
		       uint32_t speed)
{
  switch (ce) {
  case LCD6100_CE_0:
    m_raspi_ce = RASPI_CE_0;
    break;
  case LCD6100_CE_1:
    m_raspi_ce = RASPI_CE_1;
    break;
  }

  m_raspi_speed = speed;

  init_members();
 }

/////////////////////////////////////////////////////////////////////////////

lcd6100_io::~lcd6100_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::initialize(void)
{
  long rc;

  // Initialize SPI layer
  rc = raspi_initialize(m_raspi_ce,
			RASPI_MODE_3,
			RASPI_BPW_9,
			m_raspi_speed,
			0);

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to initialize SPI, SPI layer info: %s",
	      error_string);
  }

  // Initialize LCD controller
  init_lcd_controller();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::finalize(void)
{
  long rc;

  // Finalize LCD controller
  write_command(CMD_DISPOFF);
  write_command(CMD_SLEEPIN);

  // Finalize SPI layer
  rc = raspi_finalize(m_raspi_ce);

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to finalize SPI, SPI layer info: %s",
	      error_string);
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::fill_screen(LCD6100_COLOUR colour)
{
  // Drawing area is entire screen
  set_drawing_limits(0, 0,
		     LCD6100_ROW_MAX_ADDR, LCD6100_COL_MAX_ADDR);

  // Fill screen using specified RGB colour value
  // Two pixels = 2 * 12bit = 24bit = 3 bytes
  write_command(CMD_RAMWR);
  for (unsigned i=0; i < ((LCD6100_ROW_MAX_ADDR * LCD6100_COL_MAX_ADDR) / 2); i++) {
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
  uint8_t start_row = row;
  uint8_t start_col = col;
  uint8_t end_row = row + lcd_font->get_height() - 1;
  uint8_t end_col = col + lcd_font->get_width() - 1;
  
  if ( (start_row > LCD6100_ROW_MAX_ADDR) ||
       (start_col > LCD6100_COL_MAX_ADDR) ) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Start row(%u), max(%u). start col(%u), max(%u)",
		start_row, LCD6100_ROW_MAX_ADDR,
		start_col, LCD6100_COL_MAX_ADDR);
  }
  if ( (end_row > LCD6100_ROW_MAX_ADDR) ||
       (end_col > LCD6100_COL_MAX_ADDR) ) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "End row(%u), max(%u). End col(%u), max(%u)",
	      end_row, LCD6100_ROW_MAX_ADDR,
	      end_col, LCD6100_COL_MAX_ADDR);
  }

  // Drawing area is entire character
  set_drawing_limits(start_row, start_col,
		     end_row, end_col);

  // Point to last font data byte in character (last row of pixels)
  uint8_t *char_data = font_table + lcd_font->get_bytes_per_char() - 1;

  // Work through all pixels, row by row, from the bottom and up
  write_command(CMD_RAMWR);
  for (int row=0; row < lcd_font->get_height(); row++) {

    // Get pixel row from font table and then decrement row
    uint8_t pixel_row = *char_data--;
    
    // Work on each pixel in the row (left to right)
    // Note!
    // We do two pixels each loop
    uint8_t pixel_mask = 0x80;
    uint16_t pixel_0_wd;
    uint16_t pixel_1_wd;
    for (int col=0; col < lcd_font->get_width(); col+=2) {
      
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
  long rc;
  uint16_t lcd_cmd = cmd; // Bit 8 is cleared ==> command
  uint16_t rx_buf;        // Not used

  // Send command to LCD
  rc = raspi_xfer(m_raspi_ce,
		  (const void *)&lcd_cmd,
		  (void *)&rx_buf,
		  sizeof(lcd_cmd));

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to write command(0x%02x), SPI layer info: %s",
	      cmd, error_string);
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::write_data(uint8_t data)
{
  long rc;
  uint16_t lcd_data = data | 0x0100; // Bit 8 is set ==> data
  uint16_t rx_buf;                   // Not used

  // Send data to LCD
  rc = raspi_xfer(m_raspi_ce,
		  (const void *)&lcd_data,
		  (void *)&rx_buf,
		  sizeof(lcd_data));

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to write data(0x%02x), SPI layer info: %s",
	      data, error_string);
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::init_members(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::get_spi_layer_error(RASPI_ERROR_STRING error_string)
{
  RASPI_STATUS status;

  // Get / clear RASPI error information
  raspi_get_last_error(&status);
  raspi_get_error_string(status.error_code, error_string);
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io::init_lcd_controller(void)
{
  ////////////////////////////////////////////////
  // Philips PCF8833 LCD controller init sequence
  ////////////////////////////////////////////////

  // Step 1. Reset
  // JOE: Not yet implemented

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

