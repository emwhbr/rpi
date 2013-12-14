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
#include <unistd.h>
#include <sys/select.h>
#include <exception>

#include "mcp3008_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define SPI_DEV  "/dev/spidev0.0" // CE0, Raspberry Pi (Model B, GPIO P1)

#define MCP3008_REF_VOLTAGE  3.3

#define TEST_MCP3008_ERROR_MSG "*** ERROR : test_mcp3008, rc:%ld\n"

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

static void mcp3008_test_terminate(void);
static MCP3008_IO_CHANNEL get_channel_from_user(void);
static int kbhit(void);
static int getch(void);
static void initialize(void);
static void finalize(void);
static void read_single_channel(void);
static void print_menu(void);
static void do_test_mcp3008(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( mcp3008_test_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// This is the class that is tested in this application
static mcp3008_io *g_mcp3008_io = NULL;

////////////////////////////////////////////////////////////////

static void mcp3008_test_terminate(void)
{
  // Only log this event, no further actions for now
  printf("Unhandled exception, termination handler activated\n");
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static MCP3008_IO_CHANNEL get_channel_from_user(void)
{
  unsigned channel_value;
  MCP3008_IO_CHANNEL channel = MCP3008_IO_CH0;

  do {
    printf("Enter ADC channel[0..7]: ");
    scanf("%u", &channel_value);
    switch (channel_value) {
    case 0:
      channel = MCP3008_IO_CH0;
      break;
    case 1:
      channel = MCP3008_IO_CH1;
      break;
    case 2:
      channel = MCP3008_IO_CH2;
      break;
    case 3:
      channel = MCP3008_IO_CH3;
      break;
    case 4:
      channel = MCP3008_IO_CH4;
      break;
    case 5:
      channel = MCP3008_IO_CH5;
      break;
    case 6:
      channel = MCP3008_IO_CH6;
      break;
    case 7:
      channel = MCP3008_IO_CH7;
      break;
    }
  } while (channel_value > 7);

  return channel;
}

////////////////////////////////////////////////////////////////

static int kbhit(void)
{
  struct timeval tv = { 0L, 0L };
  fd_set fds;

  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}

////////////////////////////////////////////////////////////////

static int getch(void)
{
  int r;
  unsigned char c;

  if ((r = read(0, &c, sizeof(c))) < 0) {
    return r;
  } else {
    return c;
  }
}

////////////////////////////////////////////////////////////////

static void initialize(void)
{
  long rc;
  uint32_t speed;

  printf("Enter bitrate[Hz]: ");
  scanf("%u", &speed);

  rc = g_mcp3008_io->initialize(speed);
  if (rc != MCP3008_IO_SUCCESS) {
    printf(TEST_MCP3008_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void finalize(void)
{
  long rc;

  rc = g_mcp3008_io->finalize();
  if (rc != MCP3008_IO_SUCCESS) {
    printf(TEST_MCP3008_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void read_single_channel(void)
{
  long rc;
  MCP3008_IO_CHANNEL channel;
  uint16_t value;
  
  channel = get_channel_from_user();

  rc = g_mcp3008_io->read_single(channel, value);
  if (rc != MCP3008_IO_SUCCESS) {
    printf(TEST_MCP3008_ERROR_MSG, rc);
    return;
  }

  printf("ADC value: 0x%04x (dec:%04u), Volt: %.4f\n",
	 value, value, g_mcp3008_io->to_voltage(value));
}

////////////////////////////////////////////////////////////////

static void read_single_channel_dynamic(void)
{
  long rc;
  MCP3008_IO_CHANNEL channel;
  uint16_t value;
  
  channel = get_channel_from_user();

  printf("Press ENTER to quit...\n");
  while ( !kbhit() ) {

    rc = g_mcp3008_io->read_single(channel, value);
    if (rc != MCP3008_IO_SUCCESS) {
      printf(TEST_MCP3008_ERROR_MSG, rc);
      return;
    }

    printf("ADC value: 0x%04x (dec:%04u), Volt: %.4f\n",
	   value, value, g_mcp3008_io->to_voltage(value));

    sleep(1);
  }
  getch(); // Consume the character
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  printf("------------------------------------\n");
  printf("------ MCP3008 TEST MENU -----------\n");
  printf("------------------------------------\n");
  printf("\n");
  printf("  1. initialize\n");
  printf("  2. finalize\n");
  printf("  3. read single channel\n");
  printf("  4. read single channel (dynamic test)\n");
  printf("100. Exit\n\n");
}

////////////////////////////////////////////////////////////////

static void do_test_mcp3008(void)
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
      read_single_channel();
      break;
    case 4:
      read_single_channel_dynamic();
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
    g_mcp3008_io = new mcp3008_io(SPI_DEV,
				  MCP3008_REF_VOLTAGE);
    do_test_mcp3008();

    delete g_mcp3008_io;
  }
  catch (...) {
    delete g_mcp3008_io;
    throw; // Invoke termination handler
  }
  
  printf("Goodbye!\n");
  return 0;
}
