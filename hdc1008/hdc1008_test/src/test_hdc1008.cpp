// ************************************************************************
// *                                                                      *
// * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <exception>

#include "hdc1008_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define I2C_ADDRESS  0x40         // ADR0 = ADR1 = GND
#define I2C_DEV      "/dev/i2c-1" // Raspberry Pi 2 (Model B, GPIO P1)

#define TEST_HDC1008_ERROR_MSG "*** ERROR : test_hdc1008, rc:%ld\n"

#ifdef DEBUG_PRINTS
#define debug_test_printf(fmt, args...)  \
  printf("DBG - "); printf(fmt, ##args); fflush(stdout)
#else
#define debug_test_printf(fmt, args...) 
#endif // DEBUG_PRINTS

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

static void hdc1008_test_terminate(void);
static void initialize(void);
static void finalize(void);
static void read_configuration(void);
static void write_configuration(void);
static void read_temperature(void);
static void read_humidity(void);
static void print_menu(void);
static void do_test_hdc1008(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( hdc1008_test_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// This is the class that is tested in this application
static hdc1008_io *g_hdc1008_io = NULL;

////////////////////////////////////////////////////////////////

static void hdc1008_test_terminate(void)
{
  // Only log this event, no further actions for now
  printf("Unhandled exception, termination handler activated\n");
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static void initialize(void)
{
  long rc;

  rc = g_hdc1008_io->initialize();
  if (rc != HDC1008_IO_SUCCESS) {
    printf(TEST_HDC1008_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void finalize(void)
{
  long rc;

  rc = g_hdc1008_io->finalize();
  if (rc != HDC1008_IO_SUCCESS) {
    printf(TEST_HDC1008_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void read_configuration(void)
{
  long rc;
  HDC1008_IO_CFG_REG reg;

  rc = g_hdc1008_io->read_config(reg);
  if (rc != HDC1008_IO_SUCCESS) {
    printf(TEST_HDC1008_ERROR_MSG, rc);
    return;
  }
  printf("Configuration register: 0x%04x\n", reg.wd);
  printf(" res1 : 0x%02x\n", reg.bs.res1);
  printf(" hres : 0x%x\n",   reg.bs.hres);
  printf(" tres : %d\n",     reg.bs.tres);
  printf(" btst : %d\n",     reg.bs.btst);
  printf(" mode : %d\n",     reg.bs.mode);
  printf(" heat : %d\n",     reg.bs.heat);
  printf(" res2 : %d\n",     reg.bs.res2);
  printf(" rst  : %d\n",     reg.bs.rst);
}

////////////////////////////////////////////////////////////////

static void write_configuration(void)
{
  long rc;
  unsigned reg_value;
  HDC1008_IO_CFG_REG reg;

  printf("Enter configuration register [hex]: 0x");
  scanf("%x", &reg_value);

  reg.wd = (uint16_t)reg_value;

  rc = g_hdc1008_io->write_config(reg);
  if (rc != HDC1008_IO_SUCCESS) {
    printf(TEST_HDC1008_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void read_temperature(void)
{
  long rc;
  float temp_val;

  rc = g_hdc1008_io->read_temperature(temp_val);
  if (rc != HDC1008_IO_SUCCESS) {
    printf(TEST_HDC1008_ERROR_MSG, rc);
    return;
  }

  printf("Temperature[degC] : %f\n", temp_val);
}

////////////////////////////////////////////////////////////////

static void read_humidity(void)
{
  long rc;
  float hum_val;

  rc = g_hdc1008_io->read_humidity(hum_val);
  if (rc != HDC1008_IO_SUCCESS) {
    printf(TEST_HDC1008_ERROR_MSG, rc);
    return;
  }

  printf("Relative humidity[%%] : %f\n", hum_val);
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  printf("-------------------------------------\n");
  printf("-------- HDC1008 TEST MENU ----------\n");
  printf("-------------------------------------\n");
  printf("\n");
  printf("  1. initialize\n");
  printf("  2. finalize\n");
  printf("  3. read configuration\n");
  printf("  4. write configuration\n");
  printf("  5. read temperature\n");
  printf("  6. read relative humidity\n");
  printf("100. Exit\n\n");
}

////////////////////////////////////////////////////////////////

static void do_test_hdc1008(void)
{  
  int value;

  do {
    print_menu();
    
    printf("Enter choice : ");
    scanf("%d",&value);
    
    switch(value) {
    case 1:
      initialize();
      break;
    case 2:
      finalize();
      break;
    case 3:
      read_configuration();
      break;
    case 4:
      write_configuration();
      break;
    case 5:
      read_temperature();
      break;
    case 6:
      read_humidity();
      break;
    case 100: // Exit
      break;
    default:
      printf("Illegal choice!\n");
    }
  } while (value != 100);

  return;
}

////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  try {
    g_hdc1008_io = new hdc1008_io(I2C_ADDRESS,
				  I2C_DEV);
    do_test_hdc1008();

    delete g_hdc1008_io;

  }
  catch (...) {
    delete g_hdc1008_io;
    throw; // Invoke termination handler
  }
  
  printf("Goodbye!\n");
  return 0;
}
