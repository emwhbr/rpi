// ************************************************************************
// *                                                                      *
// * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <exception>

#include "ds18b20_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Sensor serial numbers, derived from contents of dir /sys/bus/w1/devices/
#define SENSOR_1_SERIAL_NUMBER     "28-0516949b08ff"
#define SENSOR_2_SERIAL_NUMBER     "28-0215535bfbff"

#define TEST_DS18B20_ERROR_MSG "*** ERROR : test_ds18b20, rc:%ld\n"

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

static void ds18b20_test_terminate(void);
static void initialize(void);
static void finalize(void);
static void read_temperature(void);
static void print_menu(void);
static void do_test_ds18b20(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( ds18b20_test_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// This is the class that is tested in this application
static ds18b20_io *g_ds18b20_io_1 = NULL;
static ds18b20_io *g_ds18b20_io_2 = NULL;

////////////////////////////////////////////////////////////////

static void ds18b20_test_terminate(void)
{
  // Only log this event, no further actions for now
  printf("Unhandled exception, termination handler activated\n");
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static ds18b20_io *get_sensor_from_user(void)
{
  unsigned id;

  do {
    printf("Enter sensor id[1..2]: ");
    scanf("%u", &id);
  } while ( (id != 1) && (id != 2) );

  if (id ==1) {
    return g_ds18b20_io_1;
  }
  else if (id == 2) {
    return g_ds18b20_io_2;
  }
  else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////

static void initialize(void)
{
  long rc;  
  ds18b20_io *sensor = get_sensor_from_user();

  if (sensor) {
    rc = sensor->initialize();
    if (rc != DS18B20_IO_SUCCESS) {
      printf(TEST_DS18B20_ERROR_MSG, rc);
      return;
    }
  }
}

////////////////////////////////////////////////////////////////

static void finalize(void)
{
  long rc;
  ds18b20_io *sensor = get_sensor_from_user();

  if (sensor) {
    rc = sensor->finalize();
    if (rc != DS18B20_IO_SUCCESS) {
      printf(TEST_DS18B20_ERROR_MSG, rc);
      return;
    }
  }
}

////////////////////////////////////////////////////////////////

static void read_temperature(void)
{
  long rc;
  float temp_val;
  ds18b20_io *sensor = get_sensor_from_user();

  if (sensor) {
    rc = sensor->read_temperature(temp_val);
    if (rc != DS18B20_IO_SUCCESS) {
      printf(TEST_DS18B20_ERROR_MSG, rc);
      return;
    }
    printf("Temperature[degC] : %f\n", temp_val);
  }  
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  printf("-------------------------------------\n");
  printf("-------- DS18B20 TEST MENU ----------\n");
  printf("-------------------------------------\n");
  printf("Sensor 1 : %s\n", SENSOR_1_SERIAL_NUMBER);
  printf("Sensor 2 : %s\n", SENSOR_2_SERIAL_NUMBER);
  printf("\n");
  printf("  1. initialize\n");
  printf("  2. finalize\n");
  printf("  3. read temperature\n");
  printf("100. Exit\n\n");
}

////////////////////////////////////////////////////////////////

static void do_test_ds18b20(void)
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
    g_ds18b20_io_1 = new ds18b20_io(SENSOR_1_SERIAL_NUMBER);
    g_ds18b20_io_2 = new ds18b20_io(SENSOR_2_SERIAL_NUMBER);
    do_test_ds18b20();
    delete g_ds18b20_io_1;
    delete g_ds18b20_io_2;
  }
  catch (...) {
    delete g_ds18b20_io_1;
    delete g_ds18b20_io_2;
    throw; // Invoke termination handler
  }
  
  printf("Goodbye!\n");
  return 0;
}
