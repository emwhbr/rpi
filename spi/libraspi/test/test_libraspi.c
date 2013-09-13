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

#include "raspi.h"

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
#define TEST_LIBRASPI_ERROR_MSG "*** ERROR : test_libraspi\n"

#define G_SPI_BUFFER_SIZE (256)

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
uint8_t *g_tx_buf = NULL;
uint8_t *g_rx_buf = NULL;

/*
 * ---------------------------------
 *       Function prototypes
 * ---------------------------------
 */
static void get_prod_info(void);
static void get_last_error(void);
static void initialize(void);
static void finalize(void);
static void xfer(void);
static void do_test_libraspi(void);


/*****************************************************************/

static void get_prod_info(void)
{
  RASPI_LIB_PROD_INFO prod_info;

  if (raspi_test_get_lib_prod_info(&prod_info) != RASPI_SUCCESS) {
    printf(TEST_LIBRASPI_ERROR_MSG);
    return;
  }
  printf("LIBRASPI prod num: %s\n", prod_info.prod_num);
  printf("LIBRASPI rstate  : %s\n", prod_info.rstate);
}

/*****************************************************************/

static void get_last_error(void)
{
  RASPI_STATUS status;
  RASPI_ERROR_STRING error_string;

  if (raspi_get_last_error(&status) != RASPI_SUCCESS) {
    printf(TEST_LIBRASPI_ERROR_MSG);
    return;
  }

  if (raspi_get_error_string(status.error_code,
			     error_string) != RASPI_SUCCESS) {
    printf(TEST_LIBRASPI_ERROR_MSG);
    return;
  }

  switch (status.error_source) {
  case RASPI_INTERNAL_ERROR:
    printf("LIBRASPI error source : RASPI_INTERNAL_ERROR\n");
    break;
  case RASPI_LINUX_ERROR:
    printf("LIBRASPI error source : RASPI_LINUX_ERROR\n");
    break;
  default:
    printf("LIBRASPI error source : *** UNKNOWN\n");
  }
  printf("LIBRASPI error code   : %ld\n", status.error_code);
  printf("LIBRASPI error string : %s\n",  error_string);
}

/*****************************************************************/

static void initialize(void)
{
  unsigned ce_value;
  unsigned mode_value;
  unsigned bpw_value;
  RASPI_CE   ce   = RASPI_CE_0;
  RASPI_MODE mode = RASPI_MODE_0;
  RASPI_BPW  bpw  = RASPI_BPW_8;
  uint32_t speed;
  unsigned i;

  /* User input */
  do {
    printf("Enter CE[0..1]: ");
    scanf("%u", &ce_value);
    switch (ce_value) {
    case 0:
      ce = RASPI_CE_0;
      break;
    case 1:
      ce = RASPI_CE_1;
      break;
    }
  } while (ce_value > 1);

  do {
    printf("Enter SPI mode[0..3]: ");
    scanf("%u", &mode_value);
    switch (mode_value) {
    case 0:
      mode = RASPI_MODE_0;
      break;
    case 1:
      mode = RASPI_MODE_1;
      break;
    case 2:
      mode = RASPI_MODE_2;
      break;
    case 3:
      mode = RASPI_MODE_1;
      break;
    }
  } while (mode_value > 3);

  do {
    printf("Enter bits/word[8..9]: ");
    scanf("%u", &bpw_value);
    switch (bpw_value) {
    case 8:
      bpw = RASPI_BPW_8;
      break;
    case 9:
      bpw = RASPI_BPW_9;
      break;
    }
  } while ( (bpw_value < 8) || (bpw_value > 9) );

  printf("Enter bitrate[Hz]: ");
  scanf("%u", &speed);
    
  /* Do initialization */
  if (raspi_initialize(ce,
		       mode,
		       bpw,
		       speed) != RASPI_SUCCESS) {
    printf(TEST_LIBRASPI_ERROR_MSG);
    return;
  }

  /* Allocate global buffers */
  g_tx_buf = malloc(G_SPI_BUFFER_SIZE);
  g_rx_buf = malloc(G_SPI_BUFFER_SIZE);

  /* Initialize transmit buffer */
  for (i=0; i < G_SPI_BUFFER_SIZE; i++) {
    g_tx_buf[i] = 0xff - (uint8_t)i;
  }

  /* Clear receive buffer */
  bzero((void *)g_rx_buf, G_SPI_BUFFER_SIZE);
}

/*****************************************************************/

static void finalize(void)
{
  unsigned ce_value;
  RASPI_CE ce = RASPI_CE_0;

  /* User input */
  do {
    printf("Enter CE[0..1]: ");
    scanf("%u", &ce_value);
    switch (ce_value) {
    case 0:
      ce = RASPI_CE_0;
      break;
    case 1:
      ce = RASPI_CE_1;
      break;
    }
  } while (ce_value > 1);

  /* Free global buffers */
  if (g_tx_buf) {
    free(g_tx_buf);
  }
  if (g_rx_buf) {
    free(g_rx_buf);
  }

  /* Do finalization */
  if (raspi_finalize(ce) != RASPI_SUCCESS) {
    printf(TEST_LIBRASPI_ERROR_MSG);
    return;
  }
}

/*****************************************************************/

static void xfer(void)
{
  unsigned ce_value;
  RASPI_CE ce = RASPI_CE_0;
  uint32_t nbytes;
  unsigned i;
  
  /* User input */  
  do {
    printf("Enter CE[0..1]: ");
    scanf("%u", &ce_value);
    switch (ce_value) {
    case 0:
      ce = RASPI_CE_0;
      break;
    case 1:
      ce = RASPI_CE_1;
      break;
    }
  } while (ce_value > 1);
  
  do {
    printf("Enter bytes to transfer[0..%d]: ", G_SPI_BUFFER_SIZE);
    scanf("%u", &nbytes);
  } while (nbytes > G_SPI_BUFFER_SIZE);

  /* Clear receive buffer */
  bzero((void *)g_rx_buf, G_SPI_BUFFER_SIZE);

  /* Do transfer */
  if (raspi_xfer(ce,
		 (const void *)g_tx_buf,
		 (void *)g_rx_buf,
		 nbytes) != RASPI_SUCCESS) {
    printf(TEST_LIBRASPI_ERROR_MSG);
    return;
  }

  /* Print receive buffer */
  printf("RX-Buffer, %u bytes:\n", nbytes);
  for (i=0; i < nbytes; i++) {
    if (!(i % 8)) {
      printf("\n");
    }
    printf("0x%02x ", g_rx_buf[i]);
  }
  printf("\n");
}

/*****************************************************************/

static void print_menu(void)
{
  printf("\n");
  printf("  1. get product info\n");
  printf("  2. get last error + get error string\n");
  printf("  3. initialize\n");
  printf("  4. finalize\n");
  printf("  5. xfer\n");
  printf("100. Exit\n\n");
}

/*****************************************************************/

static void do_test_libraspi(void)
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
      xfer();
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
  do_test_libraspi();

  printf("Goodbye!\n");
  return 0;
}
