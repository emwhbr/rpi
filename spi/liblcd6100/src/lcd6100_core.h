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

#ifndef __LCD6100_CORE_H__
#define __LCD6100_CORE_H__

#include <pthread.h>
#include <memory>

#include "lcd6100.h"
#include "lcd6100_exception.h"
#include "lcd6100_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_core {

public:
  lcd6100_core(void);
  ~lcd6100_core(void);

  long get_last_error(LCD6100_STATUS *status);

  long get_error_string(long error_code,
			LCD6100_ERROR_STRING error_string);

  long initialize(LCD6100_CE ce,
		  uint32_t speed);

  long finalize(void);

  long fill_screen(LCD6100_COLOUR colour);

  long draw_pixel(uint8_t row,
		  uint8_t col,
		  LCD6100_COLOUR colour);

  long draw_line(uint8_t start_row,
		 uint8_t start_col,
		 uint8_t end_row,
		 uint8_t end_col,
		 LCD6100_COLOUR colour);

  long draw_rectangle(uint8_t start_row,
		      uint8_t start_col,
		      uint8_t end_row,
		      uint8_t end_col,
		      bool filled,
		      LCD6100_COLOUR colour);

  long write_char(char c,
		  uint8_t row,
		  uint8_t col,		  
		  LCD6100_COLOUR fg_colour,
		  LCD6100_COLOUR bg_colour,
		  LCD6100_FONT font);

  long write_string(const char *str,
		    uint8_t row,
		    uint8_t col,		  
		    LCD6100_COLOUR fg_colour,
		    LCD6100_COLOUR bg_colour,
		    LCD6100_FONT font);

  long test_write_command(uint8_t cmd);

  long test_write_data(uint8_t data);

  long test_get_lib_prod_info(LCD6100_LIB_PROD_INFO *prod_info);

private:
  // Error handling information
  LCD6100_ERROR_SOURCE m_error_source;
  long                 m_error_code;
  bool                 m_last_error_read;
  pthread_mutex_t      m_error_mutex;

  // Keep track of initialization
  bool             m_initialized;
  pthread_mutex_t  m_init_mutex;

  // LCD i/o object
  auto_ptr<lcd6100_io> m_lcd_io_auto;

  // Private member functions
  long set_error(lcd6100_exception lxp);
  long update_error(lcd6100_exception lxp);

  long internal_get_error_string(long error_code,
				 LCD6100_ERROR_STRING error_string);

  long internal_test_get_lib_prod_info(LCD6100_LIB_PROD_INFO *prod_info);

  void internal_initialize(LCD6100_CE ce,
			   uint32_t speed);

  void internal_finalize(void);
};

#endif // __LCD6100_CORE_H__
