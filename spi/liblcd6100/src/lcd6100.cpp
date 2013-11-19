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

#include "lcd6100.h"
#include "lcd6100_core.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static lcd6100_core g_object;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long lcd6100_get_last_error(LCD6100_STATUS *status)
{
  return g_object.get_last_error(status);
}

////////////////////////////////////////////////////////////////

long lcd6100_get_error_string(long error_code,
			      LCD6100_ERROR_STRING error_string)
{
  return g_object.get_error_string(error_code, error_string);  
}

////////////////////////////////////////////////////////////////

long lcd6100_initialize(LCD6100_IFACE iface,
			uint8_t hw_reset_pin,
			LCD6100_CE ce,
			uint32_t speed)
{
  return g_object.initialize(iface, hw_reset_pin, ce, speed);
}

////////////////////////////////////////////////////////////////

long lcd6100_finalize(void)
{
  return g_object.finalize();
}

////////////////////////////////////////////////////////////////

long lcd6100_clear_screen(void)
{
  return g_object.clear_screen();
}

////////////////////////////////////////////////////////////////

long lcd6100_fill_screen(LCD6100_COLOUR colour)
{
  return g_object.fill_screen(colour);
}

////////////////////////////////////////////////////////////////

long lcd6100_draw_pixel(uint8_t row,
			uint8_t col,
			LCD6100_COLOUR colour)
{
  return g_object.draw_pixel(row, col, colour);
}

////////////////////////////////////////////////////////////////

long lcd6100_draw_line(uint8_t start_row,
		       uint8_t start_col,
		       uint8_t end_row,
		       uint8_t end_col,
		       LCD6100_COLOUR colour)
{
  return g_object.draw_line(start_row, start_col,
			    end_row, end_col,
			    colour);
}

////////////////////////////////////////////////////////////////

long lcd6100_draw_rectangle(uint8_t start_row,
			    uint8_t start_col,
			    uint8_t end_row,
			    uint8_t end_col,
			    bool filled,
			    LCD6100_COLOUR colour)
{
  return g_object.draw_rectangle(start_row, start_col,
				 end_row, end_col,
				 filled,
				 colour);
}

////////////////////////////////////////////////////////////////

long lcd6100_draw_circle(uint8_t row,
			 uint8_t col,
			 uint8_t radius,
			 LCD6100_COLOUR colour)
{
  return g_object.draw_circle(row, col, radius, colour);
}

////////////////////////////////////////////////////////////////

long lcd6100_draw_bmp_image(uint8_t row,
			    uint8_t col,
			    const char *bmp_image,
			    bool scale)
{
  return g_object.draw_bmp_image(row, col,
				 bmp_image,
				 scale);
}

////////////////////////////////////////////////////////////////

long lcd6100_write_char(char c,
			uint8_t row,
			uint8_t col,			
			LCD6100_COLOUR fg_colour,
			LCD6100_COLOUR bg_colour,
			LCD6100_FONT font)
{
  return g_object.write_char(c,
			     row, col,
			     fg_colour, bg_colour,
			     font);
}

////////////////////////////////////////////////////////////////

long lcd6100_write_string(const char *str,
			  uint8_t row,
			  uint8_t col,			
			  LCD6100_COLOUR fg_colour,
			  LCD6100_COLOUR bg_colour,
			  LCD6100_FONT font)
{
  return g_object.write_string(str,
			       row, col,
			       fg_colour, bg_colour,
			       font);
}

////////////////////////////////////////////////////////////////

long lcd6100_test_write_command(uint8_t cmd)
{
  return g_object.test_write_command(cmd);
}

////////////////////////////////////////////////////////////////

long lcd6100_test_write_data(uint8_t data)
{
  return g_object.test_write_data(data);
}

////////////////////////////////////////////////////////////////

long lcd6100_test_get_lib_prod_info(LCD6100_LIB_PROD_INFO *prod_info)
{
  return g_object.test_get_lib_prod_info(prod_info);
}
