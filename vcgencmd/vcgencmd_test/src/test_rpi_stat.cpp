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

#include "rpi_stat.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define TEST_ERROR_MSG "*** ERROR : test_rpi_stat, rc:%ld\n"

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
static void test_terminate(void);
static void read_temperature(void);
static void read_voltage(void);
static void read_frequency(void);
static void print_menu(void);
static void do_test(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( test_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// This is the class that is tested in this application
static rpi_stat *g_rpi_stat = NULL;

////////////////////////////////////////////////////////////////

static void test_terminate(void)
{
  // Only log this event, no further actions for now
  printf("Unhandled exception, termination handler activated\n");
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static void read_temperature(void)
{
  long rc;
  float temp_val;  

  rc = g_rpi_stat->get_temperature(temp_val);
  if (rc != RPI_STAT_SUCCESS) {
    printf(TEST_ERROR_MSG, rc);
    return;
  }

  printf("Temperature = %f\n", temp_val);
}

////////////////////////////////////////////////////////////////

static void read_voltage(void)
{
  long rc;
  unsigned id_val;
  RPI_STAT_VOLT_ID volt_id = RPI_STAT_VOLT_ID_CORE;
  float volt_val; 

  printf("Voltage id[0=core, 1=sdram_c, 2=sdram_i, 3=sdram_p]: ");
  scanf("%u", &id_val);

  switch (id_val) {
  case 0:
    volt_id = RPI_STAT_VOLT_ID_CORE;
    break;
  case 1:
    volt_id = RPI_STAT_VOLT_ID_SDRAM_C;
    break;
  case 2:
    volt_id = RPI_STAT_VOLT_ID_SDRAM_I;
    break;
  case 3:
    volt_id = RPI_STAT_VOLT_ID_SDRAM_P;
    break;
  default:
    printf("*** ERROR : Illegal choice\n");
    return;
  }

  rc = g_rpi_stat->get_voltage(volt_id, volt_val);
  if (rc != RPI_STAT_SUCCESS) {
    printf(TEST_ERROR_MSG, rc);
    return;
  }

  printf("Voltage = %f\n", volt_val);
}

////////////////////////////////////////////////////////////////

static void read_frequency(void)
{
  long rc;
  unsigned id_val;
  RPI_STAT_FREQ_ID freq_id = RPI_STAT_FREQ_ID_CORE;
  unsigned freq_val; 

  printf("Frequency id[0=arm,   1=core,  2=h264,  3=isp,\n");
  printf("             4=v3d,   5=uart,  6=pwm,   7=emmc,\n");
  printf("             8=pixel, 9=vec,  10=hdmi, 11=dpi]: ");
  scanf("%u", &id_val);

  switch (id_val) {
  case 0:
    freq_id = RPI_STAT_FREQ_ID_ARM;
    break;
  case 1:
    freq_id = RPI_STAT_FREQ_ID_CORE;
    break;
  case 2:
    freq_id = RPI_STAT_FREQ_ID_H264;
    break;
  case 3:
    freq_id = RPI_STAT_FREQ_ID_ISP;
    break;
  case 4:
    freq_id = RPI_STAT_FREQ_ID_V3D;
    break;
  case 5:
    freq_id = RPI_STAT_FREQ_ID_UART;
    break;
  case 6:
    freq_id = RPI_STAT_FREQ_ID_PWM;
    break;
  case 7:
    freq_id = RPI_STAT_FREQ_ID_EMMC;
    break;
  case 8:
    freq_id = RPI_STAT_FREQ_ID_PIXEL;
    break;
  case 9:
    freq_id = RPI_STAT_FREQ_ID_VEC;
    break;
  case 10:
    freq_id = RPI_STAT_FREQ_ID_HDMI;
    break;
  case 11:
    freq_id = RPI_STAT_FREQ_ID_DPI;
    break;
  default:
    printf("*** ERROR : Illegal choice\n");
    return;
  }

  rc = g_rpi_stat->get_frequency(freq_id, freq_val);
  if (rc != RPI_STAT_SUCCESS) {
    printf(TEST_ERROR_MSG, rc);
    return;
  }

  printf("Frequency = %u\n", freq_val);
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  printf("-----------------------------------\n");
  printf("--------- TEST MENU ---------------\n");
  printf("-----------------------------------\n");
  printf("\n");
  printf("  1. read temperature\n");
  printf("  2. read voltage\n");
  printf("  3. read frequency\n");
  printf("100. Exit\n\n");
}

////////////////////////////////////////////////////////////////

static void do_test(void)
{  
  int value;

  do {
    print_menu();
    
    printf("Enter choice : ");
    scanf("%d",&value);
    
    switch (value) {
    case 1:
      read_temperature();
      break;
    case 2:
      read_voltage();
      break;
    case 3:
      read_frequency();
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
    g_rpi_stat = new rpi_stat();

    do_test();

    delete g_rpi_stat;

  }
  catch (...) {
    delete g_rpi_stat;
    throw; // Invoke termination handler
  }
  
  printf("Goodbye!\n");
  return 0;
}
