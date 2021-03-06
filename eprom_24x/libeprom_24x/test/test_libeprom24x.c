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

#include "eprom24x.h"

#ifdef DEBUG_PRINTS
#define debug_test_printf(fmt, args...)  printf("DBG - "); printf(fmt, ##args); fflush(stdout)
#else
#define debug_test_printf(fmt, args...) 
#endif /* DEBUG_PRINTS */

/*
 * ---------------------------------
 *       Macros
 * ---------------------------------
 */
#define TEST_LIBEPROM24x_ERROR_MSG "*** ERROR : test_libeprom24x\n"

#define G_BUFFER_SIZE (128 * 1024 * 1024) /* Big enough for all variants of EPROMs */

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
uint8_t *g_buffer;

/*
 * ---------------------------------
 *       Function prototypes
 * ---------------------------------
 */
static void get_prod_info(void);
static void get_last_error(void);
static void initialize(void);
static void finalize(void);
static void read_u8(void);
static void read_u16(void);
static void read_u32(void);
static void read_to_buffer(void);
static void write_u8(void);
static void write_u16(void);
static void write_u32(void);
static void write_from_buffer(void);
static void erase_chip(void);
static void do_test_libeprom24x(void);


/*****************************************************************/

static void get_prod_info(void)
{
  EPROM24x_LIB_PROD_INFO prod_info;

  if (eprom24x_test_get_lib_prod_info(&prod_info)) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }
  printf("LIBEPROM24x prod num: %s\n", prod_info.prod_num);
  printf("LIBEPROM24x rstate  : %s\n", prod_info.rstate);
}

/*****************************************************************/

static void get_last_error(void)
{
  EPROM24x_STATUS status;
  EPROM24x_ERROR_STRING error_string;

  if (eprom24x_get_last_error(&status)) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  if (eprom24x_get_error_string(status.error_code, error_string)) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  switch (status.error_source) {
  case EPROM24x_INTERNAL_ERROR:
    printf("LIBEPROM24x error source : EPROM24x_INTERNAL_ERROR\n");
    break;
  case EPROM24x_LINUX_ERROR:
    printf("LIBEPROM24x error source : EPROM24x_LINUX_ERROR\n");
    break;
  default:
    printf("LIBEPROM24x error source : *** UNKNOWN\n");
  }
  printf("LIBEPROM24x error code   : %ld\n", status.error_code);
  printf("LIBEPROM24x error string : %s\n",  error_string);
}

/*****************************************************************/

static void initialize(void)
{
  EPROM24x_DEVICE eprom_device = EPROM24x_64Kbit;
  uint8_t i2c_address = 0x50;   /* A0:A1:A2 =0 */
  char *i2c_dev = "/dev/i2c-1"; /* Raspberry Pi (Model B, GPIO P1) */

  if (eprom24x_initialize(eprom_device,
			  i2c_address,
			  i2c_dev) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  g_buffer = malloc(G_BUFFER_SIZE);
}

/*****************************************************************/

static void finalize(void)
{
  if (g_buffer) {
    free(g_buffer);
  }

  if (eprom24x_finalize() != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void read_u8(void)
{
  uint32_t addr;
  uint8_t value;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  if (eprom24x_read_u8(addr, &value) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  printf("0x%05x : 0x%02x\n", addr, value);
}

/*****************************************************************/

static void read_u16(void)
{
  uint32_t addr;
  uint16_t value;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  if (eprom24x_read_u16(addr, &value) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  printf("0x%05x : 0x%04x\n", addr, value);
}

/*****************************************************************/

static void read_u32(void)
{
  uint32_t addr;
  uint32_t value;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  if (eprom24x_read_u32(addr, &value) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  printf("0x%05x : 0x%08x\n", addr, value);
}

/*****************************************************************/

static void read_to_buffer(void)
{
  uint32_t addr;
  unsigned words;
  unsigned i, j;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  printf("Enter number of 32-bit words(dec): ");
  scanf("%d", &words);

  /* Clear buffer */
  bzero((void *)g_buffer, G_BUFFER_SIZE);

  /* Read data to buffer from EPROM */
  if (eprom24x_read(addr, (void *)g_buffer, words*4) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }

  /* Print buffer as bytes */
  uint8_t *word8 = (uint8_t *)g_buffer;
  for (i=0; i < words; i++) {
    printf("0x%05x : ", addr + i*4);
    for (j=0; j < 4; j++) {
      printf("0x%02x\t", word8[i*4 + j]);
    }
    printf("\n");
  }
}

/*****************************************************************/

static void write_u8(void)
{
  uint32_t addr;
  unsigned value;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  printf("Enter data (8 bit) to write(hex): 0x");
  scanf("%x", &value);

  if (eprom24x_write_u8(addr, (uint8_t)value) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void write_u16(void)
{
  uint32_t addr;
  unsigned value;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  printf("Enter data (16 bit) to write(hex): 0x");
  scanf("%x", &value);

  if (eprom24x_write_u16(addr, (uint16_t)value) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void write_u32(void)
{
  uint32_t addr;
  uint32_t value;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  printf("Enter data (32 bit) to write(hex): 0x");
  scanf("%x", (unsigned int *)&value);

  if (eprom24x_write_u32(addr, value) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void write_from_buffer(void)
{
  uint32_t addr;
  unsigned words;
  unsigned i;

  printf("Enter EPROM address(hex): 0x");
  scanf("%x", &addr);

  printf("Enter number of 32-bit words(dec): ");
  scanf("%d", &words);

  /* Prepare buffer */
  uint32_t *word32 = (uint32_t *)g_buffer;
  for (i=0; i < words; i++) {
    word32[i] = i;
  }

  /* Write from buffer to EPROM */
  if (eprom24x_write(addr, (const void *)g_buffer, words*4) != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void erase_chip(void)
{
  /* Erase EPROM */
  if (eprom24x_erase() != EPROM24x_SUCCESS) {
    printf(TEST_LIBEPROM24x_ERROR_MSG);
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
  printf("  5. read u8\n");
  printf("  6. read u16\n");
  printf("  7. read u32\n");
  printf("  8. read to buffer\n");
  printf("  9. write u8\n");
  printf(" 10. write u16\n");
  printf(" 11. write u32\n");
  printf(" 12. write from buffer\n");
  printf(" 13. erase chip\n");
  printf("100. Exit\n\n");
}

/*****************************************************************/

static void do_test_libeprom24x(void)
{  
  int value;

  do {
    print_menu();
    
    printf("Enter choice : ");
    scanf("%d",&value);
    
    switch(value) {
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
      read_u8();
      break;
    case 6:
      read_u16();
      break;
    case 7:
      read_u32();
      break;
    case 8:
      read_to_buffer();
      break;
    case 9:
      write_u8();
      break;
    case 10:
      write_u16();
      break;
    case 11:
      write_u32();
      break;
    case 12:
      write_from_buffer();
      break;
    case 13:
      erase_chip();
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
  do_test_libeprom24x();

  printf("Goodbye!\n");
  return 0;
}
