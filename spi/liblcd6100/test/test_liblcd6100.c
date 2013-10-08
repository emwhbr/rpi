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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>

#include "lcd6100.h"

#ifdef DEBUG_PRINTS
#define debug_test_printf(fmt, args...) \
  printf("DBG - "); printf(fmt, ##args); fflush(stdout)
#else
#define debug_test_printf(fmt, args...) 
#endif /* DEBUG_PRINTS */

/*
 * ---------------------------------
 *       Macros
 * ---------------------------------
 */
#define TEST_LIBLCD6100_ERROR_MSG "*** ERROR : test_liblcd6100\n"

/*
 * ---------------------------------
 *       Types
 * ---------------------------------
 */

/*
 * ---------------------------------
 *       Global variables
 * ---------------------------------
 */

/*
 * ---------------------------------
 *       Function prototypes
 * ---------------------------------
 */
static LCD6100_COLOUR get_user_colour(void);
static void get_prod_info(void);
static void get_last_error(void);
static void initialize(void);
static void finalize(void);
static void fill_screen(void);
static void draw_pixel(void);
static void draw_line(void);
static void draw_rectangle(void);
static void write_character(void);
static void write_string(void);
static void test_write_command(void);
static void test_write_data(void);
static void do_test_liblcd6100(void);

/*****************************************************************/

static LCD6100_COLOUR get_user_colour(void)
{
  unsigned val;
  LCD6100_COLOUR colour;

  /* User input */
  printf("Enter (R)GB[hex]: 0x");
  scanf("%x", &val);
  colour.bs.red = val;

  printf("Enter R(G)B[hex]: 0x");
  scanf("%x", &val);
  colour.bs.green = val;

  printf("Enter RG(B)[hex]: 0x");
  scanf("%x", &val);
  colour.bs.blue = val;

  return colour;
}

/*****************************************************************/

static void get_prod_info(void)
{
  LCD6100_LIB_PROD_INFO prod_info;

  if (lcd6100_test_get_lib_prod_info(&prod_info) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }
  printf("LIBLCD6100 prod num: %s\n", prod_info.prod_num);
  printf("LIBLCD6100 rstate  : %s\n", prod_info.rstate);
}

/*****************************************************************/

static void get_last_error(void)
{
  LCD6100_STATUS status;
  LCD6100_ERROR_STRING error_string;

  if (lcd6100_get_last_error(&status) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }

  if (lcd6100_get_error_string(status.error_code,
			       error_string) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }

  switch (status.error_source) {
  case LCD6100_INTERNAL_ERROR:
    printf("LIBLCD6100 error source : LCD6100_INTERNAL_ERROR\n");
    break;
  case LCD6100_LINUX_ERROR:
    printf("LIBLCD6100 error source : LCD6100_LINUX_ERROR\n");
    break;
  default:
    printf("LIBLCD6100 error source : *** UNKNOWN\n");
  }
  printf("LIBLCD6100 error code   : %ld\n", status.error_code);
  printf("LIBLCD6100 error string : %s\n",  error_string);
}

/*****************************************************************/

static void initialize(void)
{
  unsigned ce_value;
  LCD6100_CE ce = LCD6100_CE_0;
  uint32_t speed;

  /* User input */
  do {
    printf("Enter CE[0..1]: ");
    scanf("%u", &ce_value);
    switch (ce_value) {
    case 0:
      ce = LCD6100_CE_0;
      break;
    case 1:
      ce = LCD6100_CE_1;
      break;
    }
  } while (ce_value > 1);

  printf("Enter bitrate[Hz]: ");
  scanf("%u", &speed);

  /* Do initialization */
  if (lcd6100_initialize(ce, speed) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void finalize(void)
{
  /* Do finalization */
  if (lcd6100_finalize() != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void fill_screen(void)
{
  LCD6100_COLOUR colour;

  /* User input */
  colour = get_user_colour();

  /* Fill screen */
  if (lcd6100_fill_screen(colour) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }  
}

/*****************************************************************/

static void draw_pixel(void)
{
  unsigned val;
  LCD6100_COLOUR colour;
  uint8_t row;
  uint8_t col;

  /* User input */
  printf("Enter row[dec]: ");
  scanf("%u", &val);
  row = val;

  printf("Enter column[dec]: ");
  scanf("%u", &val);
  col = val;

  colour = get_user_colour();

  /* Draw pixel */
  if (lcd6100_draw_pixel(row,
			 col,
			 colour) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }  
}

/*****************************************************************/

static void draw_line(void)
{
  unsigned val;
  LCD6100_COLOUR colour;
  uint8_t start_row, end_row;
  uint8_t start_col, end_col;

  /* User input */
  printf("Enter start row[dec]: ");
  scanf("%u", &val);
  start_row = val;

  printf("Enter start column[dec]: ");
  scanf("%u", &val);
  start_col = val;

  printf("Enter end row[dec]: ");
  scanf("%u", &val);
  end_row = val;

  printf("Enter end column[dec]: ");
  scanf("%u", &val);
  end_col = val;

  colour = get_user_colour();

  /* Draw line */
  if (lcd6100_draw_line(start_row, start_col,
			end_row, end_col,
			colour) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }  
}

/*****************************************************************/

static void draw_rectangle(void)
{
  unsigned val;
  LCD6100_COLOUR colour;
  bool filled;
  uint8_t start_row, end_row;
  uint8_t start_col, end_col;

  /* User input */
  printf("Enter start row[dec]: ");
  scanf("%u", &val);
  start_row = val;

  printf("Enter start column[dec]: ");
  scanf("%u", &val);
  start_col = val;

  printf("Enter end row[dec]: ");
  scanf("%u", &val);
  end_row = val;

  printf("Enter end column[dec]: ");
  scanf("%u", &val);
  end_col = val;

  colour = get_user_colour();

  printf("Fill rectangle[1=Yes, 0=No]: ");
  scanf("%u", &val);
  filled = ((val == 1) ? true : false);

  /* Draw rectangle */
  if (lcd6100_draw_rectangle(start_row, start_col,
			     end_row, end_col,
			     filled,
			     colour) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }  
}

/*****************************************************************/

static void write_character(void)
{
  char c;
  unsigned val;
  uint8_t row;
  uint8_t col;
  LCD6100_COLOUR fg_colour;
  LCD6100_COLOUR bg_colour;
  LCD6100_FONT font;
 
  /* User input */
  printf("Enter character[ascii]: ");
  scanf(" %c", &c);

  printf("Enter row[dec]: ");
  scanf("%u", &val);
  row = val;

  printf("Enter column[dec]: ");
  scanf("%u", &val);
  col = val;

  printf("FOREGROUND COLOUR\n");
  fg_colour = get_user_colour();

  printf("BACKGROUND COLOUR\n");
  bg_colour = get_user_colour();

  printf("Enter font[0=Small, 1=Medium, 2=Large]: ");
  scanf("%u", &val);
  font = val;

  /* Write character */
  if (lcd6100_write_char(c,
			 row,
			 col,
			 fg_colour,
			 bg_colour,
			 font) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }  
}

/*****************************************************************/

static void write_string(void)
{
  char str[50];
  unsigned val;
  uint8_t row;
  uint8_t col;
  LCD6100_COLOUR fg_colour;
  LCD6100_COLOUR bg_colour;
  LCD6100_FONT font;
 
  /* User input */
  printf("Enter string[ascii]: ");
  scanf(" %[^\n]s", str);

  printf("Enter row[dec]: ");
  scanf("%u", &val);
  row = val;

  printf("Enter column[dec]: ");
  scanf("%u", &val);
  col = val;

  printf("FOREGROUND COLOUR\n");
  fg_colour = get_user_colour();

  printf("BACKGROUND COLOUR\n");
  bg_colour = get_user_colour();

  printf("Enter font[0=Small, 1=Medium], 2=Large]: ");
  scanf("%u", &val);
  font = val;

  /* Write string */
  if (lcd6100_write_string(str,
			   row,
			   col,
			   fg_colour,
			   bg_colour,
			   font) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }  
}

/*****************************************************************/

static void test_write_command(void)
{
  unsigned cmd;

  /* User input */
  printf("Enter command[hex]: 0x");
  scanf("%x", &cmd);

  /* Write command to LCD */
  if (lcd6100_test_write_command((uint8_t)cmd) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void test_write_data(void)
{
  unsigned data;

  /* User input */
  printf("Enter data[hex]: 0x");
  scanf("%x", &data);

  /* Write data to LCD */
  if (lcd6100_test_write_data((uint8_t)data) != LCD6100_SUCCESS) {
    printf(TEST_LIBLCD6100_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void print_menu(void)
{
  printf("\n");
  printf("  1. get product info\n");
  printf("  2. get last error + get error string\n");
  printf("  3. initialize\n");
  printf("  4. finalize\n");
  printf("  5. fill screen\n");
  printf("  6. draw pixel\n");
  printf("  7. draw line\n");
  printf("  8. draw rectangle\n");
  printf("  9. write character\n");
  printf(" 10. write string\n");
  printf(" 11  (test) write command\n");
  printf(" 12. (test) write data\n");
  printf("100. Exit\n\n");
}

/*****************************************************************/

static void do_test_liblcd6100(void)
{  
  int value;

  do {
    print_menu();
    
    printf("Enter choice : ");
    scanf("%d",&value);
    
    switch (value) {
    case 1:
      get_prod_info();
      break;
    case 2:
      get_last_error();
      break;
    case 3:
      initialize();
      break;
    case 4:
      finalize();
      break;
    case 5:
      fill_screen();
      break;
    case 6:
      draw_pixel();
      break;
    case 7:
      draw_line();
      break;
    case 8:
      draw_rectangle();
      break;
    case 9:
      write_character();
      break;
    case 10:
      write_string();
      break;
    case 11:
      test_write_command();
      break;
    case 12:
      test_write_data();
      break;
    case 100: /* Exit */
      break;
    default:
      printf("Illegal choice!\n");
    }
  } while (value != 100);

  return;
}

/*****************************************************************/

int main(int argc, char *argv[])
{
  do_test_liblcd6100();
  
  printf("Goodbye!\n");
  return 0;
}
