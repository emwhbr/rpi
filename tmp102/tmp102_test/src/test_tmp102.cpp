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
#include <stdlib.h>
#include <stdint.h>
#include <exception>

#include "tmp102_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define I2C_ADDRESS  0x48         // ADD0 = GND
#define I2C_DEV      "/dev/i2c-1" // Raspberry Pi (Model B, GPIO P1)

#define TEST_TMP102_ERROR_MSG "*** ERROR : test_tmp102, rc:%ld\n"

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

static void tmp102_test_terminate(void);
static void initialize(void);
static void finalize(void);
static void read_configuration(void);
static void write_configuration(void);
static void read_temperature(void);
static void print_menu(void);
static void do_test_tmp102(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( tmp102_test_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// This is the class that is tested in this application
static tmp102_io *g_tmp102_io = NULL;

////////////////////////////////////////////////////////////////

static void tmp102_test_terminate(void)
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
  int value;

  printf("Extended mode [1=yes, 0=no]: ");
  scanf("%d", &value);

  bool extended_mode = (value ? true : false);

  rc = g_tmp102_io->initialize(extended_mode);
  if (rc != TMP102_IO_SUCCESS) {
    printf(TEST_TMP102_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void finalize(void)
{
  long rc;

  rc = g_tmp102_io->finalize();
  if (rc != TMP102_IO_SUCCESS) {
    printf(TEST_TMP102_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void read_configuration(void)
{
  long rc;
  TMP102_IO_CFG_REG reg;

  rc = g_tmp102_io->read_config(reg);
  if (rc != TMP102_IO_SUCCESS) {
    printf(TEST_TMP102_ERROR_MSG, rc);
    return;
  }
  printf("Configuration word: %04x\n", reg.wd);
  printf("spare: %d\n", reg.bs.spare);
  printf("em   : %d\n", reg.bs.em);
  printf("al   : %d\n", reg.bs.al);
  printf("cr0  : %d\n", reg.bs.cr0);
  printf("cr1  : %d\n", reg.bs.cr1);
  printf("sd   : %d\n", reg.bs.sd);
  printf("tm   : %d\n", reg.bs.tm);
  printf("pol  : %d\n", reg.bs.pol);
  printf("f0   : %d\n", reg.bs.f0);
  printf("f1   : %d\n", reg.bs.f1);
  printf("r0   : %d\n", reg.bs.r0);
  printf("r1   : %d\n", reg.bs.r1);
  printf("os   : %d\n", reg.bs.os);
}

////////////////////////////////////////////////////////////////

static void write_configuration(void)
{
  long rc;
  unsigned reg_value;
  TMP102_IO_CFG_REG reg;

  printf("Enter configuration word [hex]: 0x");
  scanf("%x", &reg_value);

  reg.wd = (uint16_t)reg_value;

  rc = g_tmp102_io->write_config(reg);
  if (rc != TMP102_IO_SUCCESS) {
    printf(TEST_TMP102_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void read_temperature(void)
{
  long rc;
  float temp_val;

  rc = g_tmp102_io->read_temperature(temp_val);
  if (rc != TMP102_IO_SUCCESS) {
    printf(TEST_TMP102_ERROR_MSG, rc);
    return;
  }

  printf("Temperature = %f\n", temp_val);
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  printf("-----------------------------------\n");
  printf("------ TMP102 TEST MENU -----------\n");
  printf("-----------------------------------\n");
  printf("\n");
  printf("  1. initialize\n");
  printf("  2. finalize\n");
  printf("  3. read configuration\n");
  printf("  4. write configuration\n");
  printf("  5. read temperature\n");
  printf("100. Exit\n\n");
}

////////////////////////////////////////////////////////////////

static void do_test_tmp102(void)
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
    g_tmp102_io = new tmp102_io(I2C_ADDRESS,
				I2C_DEV);
    do_test_tmp102();

    delete g_tmp102_io;

  }
  catch (...) {
    delete g_tmp102_io;
    throw; // Invoke termination handler
  }
  
  printf("Goodbye!\n");
  return 0;
}
