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

#include "gpio.h"
#include "timer.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define TEST_GPIO_ERROR_MSG "*** ERROR : test_gpio, rc:%ld\n"

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

static void test_gpio_terminate(void);
static uint8_t get_pin_from_user(void);
static void initialize(void);
static void finalize(void);
static void set_function(void);
static void get_function(void);
static void gpio_write(void);
static void gpio_read(void);
static void toggle(void);
static void print_menu(void);
static void do_test_gpio(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( test_gpio_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

// This is the class that is tested in this application
static gpio *g_gpio = NULL;

////////////////////////////////////////////////////////////////

static void test_gpio_terminate(void)
{
  // Only log this event, no further actions for now
  printf("Unhandled exception, termination handler activated\n");
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static uint8_t get_pin_from_user(void)
{
  unsigned pin;
  printf("Enter GPIO pin[0..54]: ");
  scanf("%u", &pin);

  return (uint8_t) pin;
}

////////////////////////////////////////////////////////////////

static void initialize(void)
{
  long rc;

  rc = g_gpio->initialize();
  if (rc != GPIO_SUCCESS) {
    printf(TEST_GPIO_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void finalize(void)
{
  long rc;

  rc = g_gpio->finalize();
  if (rc != GPIO_SUCCESS) {
    printf(TEST_GPIO_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void set_function(void)
{
  long rc;
  uint8_t pin;
  unsigned func_value;
  GPIO_FUNCTION func;

  // User input
  pin = get_pin_from_user();

  do {
    printf("Enter GPIO function[0..7]: ");
    scanf("%u", &func_value);
  } while ( (func_value > 7) );

  func = (GPIO_FUNCTION) func_value;

  rc = g_gpio->set_function(pin, func);
  if (rc != GPIO_SUCCESS) {
    printf(TEST_GPIO_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void get_function(void)
{
  long rc;
  uint8_t pin;
  GPIO_FUNCTION func;

  // User input
  pin = get_pin_from_user();

  rc = g_gpio->get_function(pin, func);
  if (rc != GPIO_SUCCESS) {
    printf(TEST_GPIO_ERROR_MSG, rc);
    return;
  }

  switch (func) {
  case GPIO_FUNC_INP:
    printf("Pin:%u is INPUT\n", pin);
    break;
  case GPIO_FUNC_OUT:
    printf("Pin:%u is OUTPUT\n", pin);
    break;
  case GPIO_FUNC_ALT5:
    printf("Pin:%u is ALTERNATE FUNCTION 5\n", pin);
    break;
  case GPIO_FUNC_ALT4:
    printf("Pin:%u is ALTERNATE FUNCTION 4\n", pin);
    break;
  case GPIO_FUNC_ALT0:
    printf("Pin:%u is ALTERNATE FUNCTION 0\n", pin);
    break;
  case GPIO_FUNC_ALT1:
    printf("Pin:%u is ALTERNATE FUNCTION 1\n", pin);
    break;
  case GPIO_FUNC_ALT2:
    printf("Pin:%u is ALTERNATE FUNCTION 2\n", pin);
    break;
  case GPIO_FUNC_ALT3:
    printf("Pin:%u is ALTERNATE FUNCTION 3\n", pin);
    break;
  default:
    printf("\n");
  }
}

////////////////////////////////////////////////////////////////

static void gpio_write(void)
{
  long rc;
  uint8_t pin;
  unsigned pin_value;
  uint8_t value;

  // User input
  pin = get_pin_from_user();

  printf("Enter GPIO pin value[0/1]: ");
  scanf("%u", &pin_value);

  value = (pin_value == 0) ? 0 : 1;

  rc = g_gpio->write(pin, value);
  if (rc != GPIO_SUCCESS) {
    printf(TEST_GPIO_ERROR_MSG, rc);
    return;
  }
}

////////////////////////////////////////////////////////////////

static void gpio_read(void)
{
  long rc;
  uint8_t pin;
  uint8_t value;

  // User input
  pin = get_pin_from_user();

  rc = g_gpio->read(pin, value);
  if (rc != GPIO_SUCCESS) {
    printf(TEST_GPIO_ERROR_MSG, rc);
    return;
  }

  printf("Pin:%u = %u\n", pin, value);
}

////////////////////////////////////////////////////////////////

static void toggle(void)
{
  uint8_t pin;
  unsigned nr_toggles;
  unsigned usec_delay;

  // User input
  pin = get_pin_from_user();

  printf("Enter times to toggle[dec]: ");
  scanf("%u", &nr_toggles);

  printf("Enter toggle delay time[usec]: ");
  scanf("%u", &usec_delay);

  // Save pin function
  GPIO_FUNCTION func1;
  g_gpio->get_function(pin, func1);

  // Set pin to OUT
  g_gpio->set_function(pin, GPIO_FUNC_OUT);

  // Start high
  g_gpio->write(pin, 1);

  // Toggle
  for (unsigned i=0; i < nr_toggles; i++) {

    timer t;

    // High
    g_gpio->write(pin, 0);
    t.reset();
    while (t.get_elapsed_time() < (usec_delay / 1000000.0)) ;

    // Low
    g_gpio->write(pin, 1);
    t.reset();
    while (t.get_elapsed_time() < (usec_delay / 1000000.0)) ;
  }

  // End high
  g_gpio->write(pin, 1);

  // Restore pin function
  g_gpio->set_function(pin, func1);
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  printf("---------------------------------\n");
  printf("------ GPIO TEST MENU -----------\n");
  printf("---------------------------------\n");
  printf("\n");
  printf("  1. initialize\n");
  printf("  2. finalize\n");
  printf("  3. set function\n");
  printf("  4. get function\n");
  printf("  5. write\n");
  printf("  6. read\n");
  printf("  7. toggle\n");
  printf("100. Exit\n\n");
}

////////////////////////////////////////////////////////////////

static void do_test_gpio(void)
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
      set_function();
      break;
    case 4:
      get_function();
      break;
    case 5:
      gpio_write();
      break;
    case 6:
      gpio_read();
      break;
    case 7:
      toggle();
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
    g_gpio = new gpio();

    do_test_gpio();

    delete g_gpio;
  }
  catch (...) {
    delete g_gpio;
    throw; // Invoke termination handler
  }
  
  printf("Goodbye!\n");
  return 0;
}
