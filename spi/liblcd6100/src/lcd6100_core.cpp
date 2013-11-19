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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sstream>
#include <iomanip>

#include "lcd6100_core.h"
#include "lcd6100_exception.h"
#include "lcd6100_io_raspi.h"
#include "lcd6100_io_bitbang.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PRODUCT_NUMBER   "LIBLCD6100"
#define RSTATE           "R1A08"

#define MUTEX_LOCK(mutex) \
  ({ if (pthread_mutex_lock(&mutex)) { \
      return LCD6100_MUTEX_FAILURE; \
    } })

#define MUTEX_UNLOCK(mutex) \
  ({ if (pthread_mutex_unlock(&mutex)) { \
      return LCD6100_MUTEX_FAILURE; \
    } })

#ifdef DEBUG_PRINTS
//
// Notes!
// Macro 'debug_printf' can be used anywhere in LIBLCD6100.
// The other macros can only be used in function 'update_error'.
//
#define debug_printf(fmt, args...)  printf("LIBLCD6100 - "); \
                                    printf(fmt, ##args); \
				    fflush(stdout)

#define debug_linux_error()         printf("LIBLCD6100 LINUX ERROR - "); \
                                    error(0, errno, NULL); \
				    fflush(stdout)

#define debug_internal_error()      printf("LIBLCD6100 INTERNAL ERROR\n"); \
				    fflush(stdout)
#else
#define debug_printf(fmt, args...) 
#define debug_linux_error()
#define debug_internal_error()
#endif // DEBUG_PRINTS

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_core::lcd6100_core(void)
{
  m_error_source    = LCD6100_INTERNAL_ERROR;
  m_error_code      = 0;
  m_last_error_read = true;
  pthread_mutex_init(&m_error_mutex, NULL); // Use default mutex attributes

  m_initialized = false;
  pthread_mutex_init(&m_init_mutex, NULL);  // Use default mutex attributes

  m_lcd_io_auto.reset();
}

/////////////////////////////////////////////////////////////////////////////

lcd6100_core::~lcd6100_core(void)
{
  pthread_mutex_destroy(&m_error_mutex);
  pthread_mutex_destroy(&m_init_mutex);
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::get_last_error(LCD6100_STATUS *status)
{
  try {
    MUTEX_LOCK(m_error_mutex);
    status->error_source = m_error_source;
    status->error_code   = m_error_code;
    
    // Clear internal error information
    m_error_source    = LCD6100_INTERNAL_ERROR;
    m_error_code      = LCD6100_NO_ERROR;
    m_last_error_read = true;
    MUTEX_UNLOCK(m_error_mutex);
    return LCD6100_SUCCESS;
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::get_error_string(long error_code, 
				    LCD6100_ERROR_STRING error_string)
{
  try {
    // Do the actual work
    return internal_get_error_string(error_code, error_string);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::initialize(LCD6100_IFACE iface,
			      uint8_t hw_reset_pin,
			      LCD6100_CE ce,
			      uint32_t speed)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if already initialized
    if (m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_ALREADY_INITIALIZED,
		"Already initialized, ce=%d", ce);
    }

    // Do the actual initialization
    internal_initialize(iface, hw_reset_pin, ce, speed);

    // Initialization completed
    m_initialized = true;
    MUTEX_UNLOCK(m_init_mutex);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(lxp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::finalize(void)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual finalization
    internal_finalize();

    // Finalization completed
    m_initialized = false;
    MUTEX_UNLOCK(m_init_mutex);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(lxp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::clear_screen(void)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    LCD6100_COLOUR colour;
    colour.wd = LCD6100_RGB_BLACK;

    // Do the actual work
    m_lcd_io_auto->fill_screen(colour);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::fill_screen(LCD6100_COLOUR colour)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    m_lcd_io_auto->fill_screen(colour);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::draw_pixel(uint8_t row,
			      uint8_t col,
			      LCD6100_COLOUR colour)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if ( (row > LCD6100_ROW_MAX_ADDR) ||
	 (col > LCD6100_COL_MAX_ADDR) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Row(%u), max(%u). Col(%u), max(%u)",
		row, LCD6100_ROW_MAX_ADDR,
		col, LCD6100_COL_MAX_ADDR);
    }

    // Do the actual work
    m_lcd_io_auto->draw_pixel(row, col, colour);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::draw_line(uint8_t start_row,
			     uint8_t start_col,
			     uint8_t end_row,
			     uint8_t end_col,
			     LCD6100_COLOUR colour)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
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
    
    // Do the actual work
    m_lcd_io_auto->draw_line(start_row, start_col,
			     end_row, end_col,
			     colour);
    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::draw_rectangle(uint8_t start_row,
				  uint8_t start_col,
				  uint8_t end_row,
				  uint8_t end_col,
				  bool filled,
				  LCD6100_COLOUR colour)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
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
    
    // Do the actual work
    m_lcd_io_auto->draw_rectangle(start_row, start_col,
				  end_row, end_col,
				  filled,
				  colour);
    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::draw_circle(uint8_t row,
			       uint8_t col,
			       uint8_t radius,
			       LCD6100_COLOUR colour)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    int max_row = row + radius;
    int min_row = row - radius;
    if ( (max_row > LCD6100_ROW_MAX_ADDR) ||
	 (min_row < 0) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Row(%u), radius(%u). Exceeds row limits min(%u), max(%u)",
		row, radius, 0, LCD6100_ROW_MAX_ADDR);
    }
    int max_col = col + radius;
    int min_col = col - radius;
    if ( (max_col > LCD6100_COL_MAX_ADDR) ||
	 (min_col < 0) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Col(%u), radius(%u). Exceeds col limits min(%u), max(%u)",
		col, radius, 0, LCD6100_COL_MAX_ADDR);
    }

    // Do the actual work
    m_lcd_io_auto->draw_circle(row, col, radius, colour);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::draw_bmp_image(uint8_t row,
				  uint8_t col,
				  string bmp_image,
				  bool scale)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if ( (row > LCD6100_ROW_MAX_ADDR) ||
	 (col > LCD6100_COL_MAX_ADDR) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Row(%u), max(%u). Col(%u), max(%u)",
		row, LCD6100_ROW_MAX_ADDR,
		col, LCD6100_COL_MAX_ADDR);
    }

    // Do the actual work
    m_lcd_io_auto->draw_bmp_image(row, col,
				  bmp_image,
				  scale);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::write_char(char c,
			      uint8_t row,
			      uint8_t col,			      
			      LCD6100_COLOUR fg_colour,
			      LCD6100_COLOUR bg_colour,
			      LCD6100_FONT font)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if ( (row > LCD6100_ROW_MAX_ADDR) ||
	 (col > LCD6100_COL_MAX_ADDR) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Row(%u), max(%u). Col(%u), max(%u)",
		row, LCD6100_ROW_MAX_ADDR,
		col, LCD6100_COL_MAX_ADDR);
    }

    // Do the actual work
    m_lcd_io_auto->write_char(c,
			      row, col,
			      fg_colour, bg_colour,
			      font);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::write_string(const char *str,
				uint8_t row,
				uint8_t col,			      
				LCD6100_COLOUR fg_colour,
				LCD6100_COLOUR bg_colour,
				LCD6100_FONT font)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!str) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"str is null pointer", NULL);
    }

    if ( (row > LCD6100_ROW_MAX_ADDR) ||
	 (col > LCD6100_COL_MAX_ADDR) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
		"Row(%u), max(%u). Col(%u), max(%u)",
		row, LCD6100_ROW_MAX_ADDR,
		col, LCD6100_COL_MAX_ADDR);
    }

    // Do the actual work
    m_lcd_io_auto->write_string(str,
				row, col,
				fg_colour, bg_colour,
				font);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::test_write_command(uint8_t cmd)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    m_lcd_io_auto->write_command(cmd);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::test_write_data(uint8_t data)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    m_lcd_io_auto->write_data(data);

    return LCD6100_SUCCESS;
  }
  catch (lcd6100_exception &lxp) {
    return set_error(lxp);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::test_get_lib_prod_info(LCD6100_LIB_PROD_INFO *prod_info)
{
  try {
    // Do the actual work
    return internal_test_get_lib_prod_info(prod_info);
  }
  catch (...) {
    return set_error(LXP(LCD6100_INTERNAL_ERROR, LCD6100_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::set_error(lcd6100_exception lxp)
{
#ifdef DEBUG_PRINTS
  // Get the stack trace
  STACK_FRAMES frames;
  lxp.get_stack_frames(frames);

  ostringstream oss_msg;
  char buffer[18];

  oss_msg << "stack frames:" << (int) frames.active_frames << "\n";

  for (unsigned i=0; i < frames.active_frames; i++) {
    sprintf(buffer, "0x%08x", frames.frames[i]);
    oss_msg << "\tframe:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\n";
  }

  // Get info from predefined macros
  oss_msg << "\tViolator: " << lxp.get_file() 
	  << ":" << lxp.get_line()
	  << ", " << lxp.get_function() << "\n";

  // Get the internal info
  oss_msg << "\tSource: " << lxp.get_source()
	  << ", Code: " << lxp.get_code() << "\n";

  oss_msg << "\tInfo: " << lxp.get_info() << "\n";

  // Print all info
  debug_printf(oss_msg.str().c_str());
#endif

  // Update internal error information
  return update_error(lxp);
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::update_error(lcd6100_exception lxp)
{
  MUTEX_LOCK(m_error_mutex);
  if (m_last_error_read) {
    m_error_source    = lxp.get_source();
    m_error_code      = lxp.get_code();
    m_last_error_read = false; // Latch last error until read
  }
  MUTEX_UNLOCK(m_error_mutex);

#ifdef DEBUG_PRINTS 
  switch(lxp.get_source()) {
  case LCD6100_INTERNAL_ERROR:
    debug_internal_error();
    break;
  case LCD6100_LINUX_ERROR:
    debug_linux_error();
    break;
  }
#endif

  return LCD6100_FAILURE;
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::internal_get_error_string(long error_code,
					     LCD6100_ERROR_STRING error_string)
{
  size_t str_len = sizeof(LCD6100_ERROR_STRING);

  switch (error_code) {
  case LCD6100_NO_ERROR:
    strncpy(error_string, "No error", str_len);
    break;
  case LCD6100_NOT_INITIALIZED:
    strncpy(error_string, "Not initialized", str_len);
    break;
  case LCD6100_ALREADY_INITIALIZED:
    strncpy(error_string, "Already initialized", str_len);
    break;
  case LCD6100_BAD_ARGUMENT:
    strncpy(error_string, "Bad argument", str_len);
    break;
  case LCD6100_CLOCK_OPERATION_FAILED:
    strncpy(error_string, "Clock operation failed", str_len);
    break;
  case LCD6100_SPI_LAYER_ERROR:
    strncpy(error_string, "SPI layer error", str_len);
    break;
  case LCD6100_BMP_IMAGE_ERROR:
    strncpy(error_string, "BMP image error", str_len);
    break;
  case LCD6100_FILE_OPERATION_FAILED:
    strncpy(error_string, "File operation failed", str_len);
    break;
  case LCD6100_MEMORY_MAP_FAILED:
    strncpy(error_string, "Memory map failed", str_len);
    break;
  case LCD6100_UNEXPECTED_EXCEPTION:
    strncpy(error_string, "Unexpected exception", str_len);
    break;
  default: 
    strncpy(error_string, "Undefined error", str_len);
  }

  return LCD6100_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long lcd6100_core::internal_test_get_lib_prod_info(LCD6100_LIB_PROD_INFO *prod_info)
{
  long rc = LCD6100_SUCCESS;
 
  strncpy(prod_info->prod_num, 
	  PRODUCT_NUMBER, 
	  sizeof(((LCD6100_LIB_PROD_INFO *)0)->prod_num));

  strncpy(prod_info->rstate, 
	  RSTATE, 
	  sizeof(((LCD6100_LIB_PROD_INFO *)0)->rstate));

  return rc;
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_core::internal_initialize(LCD6100_IFACE iface,
				       uint8_t hw_reset_pin,
				       LCD6100_CE ce,
				       uint32_t speed)
{
  lcd6100_io *lcd_io_ptr = NULL;

  // Create the LCD i/o object with garbage collector
  if (iface == LCD6100_IFACE_BITBANG) {
    lcd_io_ptr = new lcd6100_io_bitbang(hw_reset_pin,
					ce);          // Speed is not an
                                                      // option here
  }
  else {
    lcd_io_ptr = new lcd6100_io_raspi(hw_reset_pin,
				      ce,
				      speed);
  }
  m_lcd_io_auto = auto_ptr<lcd6100_io>(lcd_io_ptr);

  // Initialize LCD i/o object
  m_lcd_io_auto->initialize();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_core::internal_finalize(void)
{
  // Finalize the LCD i/o object
  m_lcd_io_auto->finalize();

  // Delete the LCD i/o object
  m_lcd_io_auto.reset();
}
