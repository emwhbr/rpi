/************************************************************************
 *                                                                      *
 * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 ************************************************************************/

#ifndef __LCD6100_H__
#define __LCD6100_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 * LIBLCD6100 Return codes
 */
#define LCD6100_SUCCESS         0
#define LCD6100_FAILURE        -1
#define LCD6100_MUTEX_FAILURE  -2

/*
 * LIBLCD6100 Internal error codes
 */
#define LCD6100_NO_ERROR                      0
#define LCD6100_NOT_INITIALIZED               1
#define LCD6100_ALREADY_INITIALIZED           2
#define LCD6100_BAD_ARGUMENT                  3
#define LCD6100_CLOCK_OPERATION_FAILED        4
#define LCD6100_SPI_LAYER_ERROR               5
#define LCD6100_BMP_IMAGE_ERROR               6
#define LCD6100_FILE_OPERATION_FAILED         7 // JOE: Continue here
#define LCD6100_MEMORY_MAP_FAILED             8 // JOE: Continue here
#define LCD6100_UNEXPECTED_EXCEPTION          9

/*
 * Error source values
 */
typedef enum {LCD6100_INTERNAL_ERROR, 
	      LCD6100_LINUX_ERROR} LCD6100_ERROR_SOURCE;
/*
 * Flag values
 */

/*
 * Basic API support types
 */

/*
 * API types
 */

typedef char LCD6100_ERROR_STRING[256];

typedef struct {
  char prod_num[20];
  char rstate[10];
} LCD6100_LIB_PROD_INFO;

typedef struct {
  LCD6100_ERROR_SOURCE error_source;
  long               error_code;
} LCD6100_STATUS;

/* SPI chip selects */
typedef enum {LCD6100_CE_0,
              LCD6100_CE_1} LCD6100_CE;

/* Underlying SPI framework for LCD communication */
typedef enum {LCD6100_IFACE_BITBANG,
	      LCD6100_IFACE_LINUX_NATIVE} LCD6100_IFACE;

/* Fonts */
typedef enum {LCD6100_FONT_SMALL,
	      LCD6100_FONT_MEDIUM,
              LCD6100_FONT_LARGE} LCD6100_FONT;

/* 12-bit RGB : rrrrgggbbbb */
typedef union {
  struct {
    uint8_t blue     : 4;
    uint8_t green    : 4;
    uint8_t red      : 4;
    uint8_t not_used : 4;
  } __attribute__ ((packed)) bs;
  uint16_t wd;
} LCD6100_COLOUR; 

/* The LCD is a 132 x 132 pixel matrix */
#define LCD6100_ROW_MAX_ADDR  131
#define LCD6100_COL_MAX_ADDR  131

/* 
 * Colour values, 12-bit RGB, 4096 colours (rrrrggggbbbb).
 * Note!
 * Convert from a 24-bit value (3 x 8 bits) to a 12-bit value (3 x 4 bits):
 * Divide each 8-bit value [0-255] by 16 to obtain the approximate value
 * in a 4-bit value [0-15].
*/
#define LCD6100_RGB_WHITE   0xFFF
#define LCD6100_RGB_GRAY    0x888
#define LCD6100_RGB_SILVER  0xCCC
#define LCD6100_RGB_BLACK   0x000
#define LCD6100_RGB_RED     0xF00
#define LCD6100_RGB_MAROON  0x800
#define LCD6100_RGB_MAGENTA 0xF0F
#define LCD6100_RGB_PINK    0xF6A
#define LCD6100_RGB_BROWN   0x822
#define LCD6100_RGB_CHOCO   0xD61
#define LCD6100_RGB_LIME    0x0F0
#define LCD6100_RGB_GREEN   0x080
#define LCD6100_RGB_OLIVE   0x880
#define LCD6100_RGB_BLUE    0x00F
#define LCD6100_RGB_NAVY    0x008
#define LCD6100_RGB_PURPLE  0x808
#define LCD6100_RGB_CYAN    0x0FF
#define LCD6100_RGB_YELLOW  0xFF0
#define LCD6100_RGB_ORANGE  0xFA0

/****************************************************************************
*
* Name lcd6100_get_last_error
*
* Description Returns the error information held by LIBLCD6100, when a call
*             returns unsuccessful completion. 
*             LIBLCD6100 clears its internal error information after it has
*             been read by the calling application.
*
* Parameters status  IN/OUT  pointer to a buffer to hold the error information
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_get_last_error(LCD6100_STATUS *status);

/****************************************************************************
*
* Name lcd6100_get_error_string
*
* Description Returns the error string corresponding to the provided
*             internal error code.
*
* Parameters error_code    IN      Actual error code
*            error_string  IN/OUT  Pointer to a buffer to hold the error string
*
* Error handling Returns always LCD6100_SUCCESS
*
****************************************************************************/
extern long lcd6100_get_error_string(long error_code, 
				     LCD6100_ERROR_STRING error_string);

/****************************************************************************
*
* Name lcd6100_initialize
*
* Description Allocates system resources and performs operations that are
*             necessary to be able to communicate with a device using the
*             SPI interface for specified chip select.
*
*             This function shall be called once to make LIBLCD6100
*             operational for a chip select.
*
*             This function can be called again after finalization.
*
* Parameters iface         IN  Underlying SPI framework/interface.
*            hw_reset_pin  IN  GPIO pin for hardware reset of LCD chip
*            ce            IN  SPI chip select.
*            speed         IN  Bitrate (Hz).
*                              Only valid for LCD6100_IFACE_LINUX_NATIVE.
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_initialize(LCD6100_IFACE iface,
			       uint8_t hw_reset_pin,
			       LCD6100_CE ce,
			       uint32_t speed);

/****************************************************************************
*
* Name lcd6100_finalize
*
* Description Releases any resources that were claimed during initialization.
*
* Parameters None.
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_finalize(void);

/****************************************************************************
*
* Name lcd6100_clear_screen
*
* Description Clears the LCD screen.
*
* Parameters None
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_clear_screen(void);

/****************************************************************************
*
* Name lcd6100_fill_screen
*
* Description Fills the LCD screen with specified colour.
*
* Parameters colour  IN  12-bit RGB colour value (rrrrgggbbbb)
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_fill_screen(LCD6100_COLOUR colour);

/****************************************************************************
*
* Name lcd6100_draw_pixel
*
* Description Draws one pixel on LCD screen with specified colour.
*
* Parameters  row     IN  Row address
*             col     IN  Column address
*             colour  IN  12-bit RGB colour value (rrrrgggbbbb)
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_draw_pixel(uint8_t row,
			       uint8_t col,
			       LCD6100_COLOUR colour);

/****************************************************************************
*
* Name lcd6100_draw_line
*
* Description Draws a line on LCD screen with specified colour.
*
* Parameters  start_row  IN  Start row address
*             start_col  IN  Start column address
*             end_row    IN  End row address
*             end_col    IN  End column address
*             colour     IN  12-bit RGB colour value (rrrrgggbbbb)
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_draw_line(uint8_t start_row,
			      uint8_t start_col,
			      uint8_t end_row,
			      uint8_t end_col,
			      LCD6100_COLOUR colour);

/****************************************************************************
*
* Name lcd6100_draw_rectangle
*
* Description Draws a rectangle on LCD screen with specified colour.
*
* Parameters  start_row  IN  Start row address
*             start_col  IN  Start column address
*             end_row    IN  End row address
*             end_col    IN  End column address
*             filled     IN  If rectangle shall be filled or not
*             colour     IN  12-bit RGB colour value (rrrrgggbbbb)
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_draw_rectangle(uint8_t start_row,
				   uint8_t start_col,
				   uint8_t end_row,
				   uint8_t end_col,
				   bool filled,
				   LCD6100_COLOUR colour);

/****************************************************************************
*
* Name lcd6100_draw_circle
*
* Description Draws a circle on LCD screen with specified colour.
*
* Parameters  row     IN  Row address
*             col     IN  Column address
*             radius  IN  Radius of circle (in pixels)
*             colour  IN  12-bit RGB colour value (rrrrgggbbbb)
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_draw_circle(uint8_t row,
				uint8_t col,
				uint8_t radius,
				LCD6100_COLOUR colour);

/****************************************************************************
*
* Name lcd6100_draw_bmp_image
*
* Description Draws a BMP image on LCD.
*
* Parameters  row        IN  Start row address
*             col        IN  Start column address
*             bmp_image  IN  Path to BMP image
*             scale      IN  If image shall be scaled to fit or not.
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_draw_bmp_image(uint8_t row,
				   uint8_t col,
				   const char *bmp_image,
				   bool scale);

/****************************************************************************
*
* Name lcd6100_write_char
*
* Description Writes a single character on LCD screen.
*
* Parameters  c          IN  Character
*             row        IN  Row address
*             col        IN  Column address
*             fg_colour  IN  12-bit RGB foreground colour value (rrrrgggbbbb)
*             bg_colour  IN  12-bit RGB background colour value (rrrrgggbbbb)
*             font       IN  Font type
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_write_char(char c,
			       uint8_t row,
			       uint8_t col,
			       LCD6100_COLOUR fg_colour,
			       LCD6100_COLOUR bg_colour,
			       LCD6100_FONT font);

/****************************************************************************
*
* Name lcd6100_write_string
*
* Description Writes a string of characters on LCD screen.
*
* Parameters  *str       IN  Pointer to a string of characters
*             row        IN  Row address
*             col        IN  Column address
*             fg_colour  IN  12-bit RGB foreground colour value (rrrrgggbbbb)
*             bg_colour  IN  12-bit RGB background colour value (rrrrgggbbbb)
*             font       IN  Font type
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_write_string(const char *str,
				 uint8_t row,
				 uint8_t col,
				 LCD6100_COLOUR fg_colour,
				 LCD6100_COLOUR bg_colour,
				 LCD6100_FONT font);

/****************************************************************************
*
* Name lcd6100_test_write_command
*
* Description Writes a command to LCD.
*
* Parameters cmd  IN  Command to write to LCD.
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_test_write_command(uint8_t cmd);

/****************************************************************************
*
* Name lcd6100_test_write_data
*
* Description Writes data to LCD.
*
* Parameters data  IN  Data to write to LCD.
*
* Error handling Returns LCD6100_SUCCESS if successful
*                otherwise LCD6100_FAILURE or LCD6100_MUTEX_FAILURE
*
****************************************************************************/
extern long lcd6100_test_write_data(uint8_t data);

/****************************************************************************
*
* Name lcd6100_test_get_lib_prod_info
*
* Description Returns the product number and the RState of LIBLCD6100.
*
* Parameters prod_info  IN/OUT  Pointer to a buffer to hold the product
*                               number and the RState.
*
* Error handling Returns always LCD6100_SUCCESS.
*
****************************************************************************/
extern long lcd6100_test_get_lib_prod_info(LCD6100_LIB_PROD_INFO *prod_info);

#ifdef  __cplusplus
}
#endif

#endif /* __LCD6100_H__ */
