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
#include <strings.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <math.h>
#include <iostream>
#include <exception>

#include "lcd6100.h"
#include "lcd.h"
#include "delay.h"
#include "excep.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define DEMO_REVISION  "R1A04"

#define LCD_IFACE       LCD6100_IFACE_BITBANG
#define LCD_HW_RES_PIN  23  // Connector P1-16
#define LCD_CE          LCD6100_CE_0
#define LCD_SPEED       0   // Not valid for bitbang interface

/////////////////////////////////////////////////////////////////////////////
//               Definitions of types
/////////////////////////////////////////////////////////////////////////////

// Data periodically updated by controller thread
typedef struct {
  uint32_t timer_tick;
  bool     update_digital;
  bool     update_analog;
  uint8_t  digital_hour;
  uint8_t  digital_min;
  uint8_t  digital_sec;
  uint8_t  analog_sec_end_row;
  uint8_t  analog_sec_end_col;
} LCD_TIME;

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

// Thread function prototypes
static void timer_thread_func(union sigval sv);
static void *ctrl_thread_func(void *ptr);

// Non-thread function prototypes
static void lcd6100_clock_demo_terminate(void);
static void get_prod_info(void);
static void start_clock(void);
static void stop_clock(void);
static void print_menu(void);
static void clock_demo_menu(void);
static void lcd6100_clock_demo(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( lcd6100_clock_demo_terminate );
  }
};

// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

static timer_t g_timer_id;

static lcd *g_lcd = NULL;

static volatile sig_atomic_t g_run_demo = 1;
static volatile sig_atomic_t g_clock_started = 0;

static sem_t g_ctrl_sem;

static LCD_TIME g_lcd_time;
static pthread_mutex_t g_lcd_time_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool g_timer_thread_error = false;

////////////////////////////////////////////////////////////////

static void timer_thread_func(union sigval sv)
{
  // Don't run if any prevoius error in this thread
  if (g_timer_thread_error) {
    return;
  }

  // If controller got mutex here, we are in trouble and probably late!
  // But this can also happen if controller is doing periodic check.
  /*
  if (pthread_mutex_trylock(&g_lcd_time_mutex) == EBUSY) {
    cout << "WARNING: Timer thread got no time mutex, are we late?\n";
    return;
  }
  */

  pthread_mutex_lock(&g_lcd_time_mutex);

  // We got the mutex now!

  try {
    // Check that controller has updated with new data
    if (g_lcd_time.update_digital) {
      cout << "WARNING: Timer thread detected non-updated digital time\n";
    }
    if (g_lcd_time.update_analog) {
      cout << "WARNING: Timer thread detected non-updated analog time\n";
    }

    // Timer fires each 0.05 seconds
    // Digital time shall be updated once per second
    // Analog time shall always be updated
    if (g_lcd_time.timer_tick % 20 == 0) {
      g_lcd->draw_digital_time();       // Draw LCD digital time
      g_lcd_time.update_digital = true; // Signal controller thread ok
                                        // to update new data
    }

    g_lcd->draw_analog_time();       // Draw LCD analog time

    if (g_lcd_time.timer_tick % (20 * 60) == 0) {
      g_lcd->clear_analog_time();    // Clear last analog minute
    }

    g_lcd_time.update_analog = true; // Signal controller thread ok
                                     // to update new data    
  }
  catch (excep &exp) {
    cout << exp.what();
    g_timer_thread_error = true;
  }
  catch (...) {
    cout << "Unknown exception in timer thread\n";
    g_timer_thread_error = true;
  }

  pthread_mutex_unlock(&g_lcd_time_mutex);
}

////////////////////////////////////////////////////////////////

static void *ctrl_thread_func(void *ptr)
{
  try {
    // Signal that we have started
    if (sem_post(&g_ctrl_sem)) {
      THROW_EXP(0, 0, "sem_post(CTRL), failed", NULL);
    }

    while (g_run_demo) {
      
      // Take it easy
      if (delay(0.020) != DELAY_SUCCESS) {
	THROW_EXP(0, 0, "controller thread, delay() failed", NULL);
      }
      
      // No need to update if clock is not running
      if (!g_clock_started) {
	continue;
      }
      
      // Only update if timer thread has drawn old data      
      // Prepare (update) data for next drawing

      pthread_mutex_lock(&g_lcd_time_mutex);

      if (g_lcd_time.update_digital) {
	// Update digital time
	// This means that one second has passed
	if (++g_lcd_time.digital_sec == 60) {
	  g_lcd_time.digital_sec = 0;
	  if (++g_lcd_time.digital_min == 60) {
	    g_lcd_time.digital_min = 0;
	    if (++g_lcd_time.digital_hour == 24) {
	      g_lcd_time.digital_hour = 0;
	    }
	  }      
	}
	g_lcd->update_digital_time(g_lcd_time.digital_hour,
				   g_lcd_time.digital_min,
				   g_lcd_time.digital_sec);
	
	g_lcd_time.update_digital = false; // No need to update until
             	                           // timer thread has drawn this data	
      }

      if (g_lcd_time.update_analog) {
	// Update analog time
	// This means that 0.05 seconds has passed
	// Note! One second equals 6 degrees
	g_lcd_time.timer_tick++;

	const uint8_t origo_row = g_lcd->get_analog_origo_row();
	const uint8_t origo_col = g_lcd->get_analog_origo_col();
	const uint8_t r = g_lcd->get_analog_radius();
	const float   s = g_lcd_time.timer_tick * 0.05;

	g_lcd_time.analog_sec_end_row = origo_row + (r * cos(s*6*M_PI/180.0));
	g_lcd_time.analog_sec_end_col = origo_col + (r * sin(s*6*M_PI/180.0));
	
	g_lcd->update_analog_time(g_lcd_time.analog_sec_end_row,
				  g_lcd_time.analog_sec_end_col);

	g_lcd_time.update_analog = false; // No need to update until
             	                          // timer thread has drawn this data       
      }
      pthread_mutex_unlock(&g_lcd_time_mutex);
    }    
  } 
  catch (excep &exp) {
    cout << exp.what();
    pthread_mutex_unlock(&g_lcd_time_mutex);
  }
  catch (...) {
    cout << "Unknown exception in controller thread\n";
    pthread_mutex_unlock(&g_lcd_time_mutex);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////

static void lcd6100_clock_demo_terminate(void)
{
  // Only log this event, no further actions for now
  cout << "Unhandled exception, termination handler activated\n";
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static void get_prod_info(void)
{
  cout << "Demo revision      : " << DEMO_REVISION << endl;

  LCD6100_LIB_PROD_INFO lib_prod_info;
  if (lcd6100_test_get_lib_prod_info(&lib_prod_info) != LCD6100_SUCCESS) {
    THROW_EXP(0, 0, "lcd6100_test_get_lib_prod_info() failed", NULL);
    return;
  }

  cout << "libLCD6100 prod num: " << lib_prod_info.prod_num << endl; 
  cout << "libLCD6100 rstate  : " << lib_prod_info.rstate << endl;
}

////////////////////////////////////////////////////////////////

static void start_clock(void)
{
  int rc;
  struct sigevent se;
  struct itimerspec ts;

  // Check if timer running
  if (g_clock_started) {
    cout << "WARNING: Clock already started\n";
    return;
  }

  // Reset digital clock on LCD
  g_lcd->reset_digital_time();

  // Reset analog clock on LCD
  g_lcd->clear_analog_time();
  g_lcd->reset_analog_time();

  const uint8_t analog_zero_sec_row =
    g_lcd->get_analog_origo_row() + g_lcd->get_analog_radius();
  const uint8_t analog_zero_sec_col = g_lcd->get_analog_origo_col();

  // Signal controller thread ok to update new data
  pthread_mutex_lock(&g_lcd_time_mutex);

  g_lcd_time.timer_tick = 0;
  g_lcd_time.update_digital = true;
  g_lcd_time.update_analog  = true;
  g_lcd_time.digital_hour = 0;
  g_lcd_time.digital_min  = 0;
  g_lcd_time.digital_sec  = 0;
  g_lcd_time.analog_sec_end_row = analog_zero_sec_row;
  g_lcd_time.analog_sec_end_col = analog_zero_sec_col;

  pthread_mutex_unlock(&g_lcd_time_mutex);

  // Set the sigevent structure to cause the signal
  // to be delivered by creating a new thread.
  bzero(&se, sizeof(se));
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &g_timer_id;
  se.sigev_notify_function = timer_thread_func;
  se.sigev_notify_attributes = NULL;

  // Specify a repeating timer that fires each 0.25 seconds.
  // First expiration is after 1 seconds.
  ts.it_value.tv_sec = 1;
  ts.it_value.tv_nsec = 0;
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 50000000; // 0.05 s

  // Create timer
  rc = timer_create(CLOCK_REALTIME, &se, &g_timer_id);
  if (rc == -1) {
    THROW_EXP(0, 0, "timer_create() failed", NULL);
  }

  // Start timer
  rc = timer_settime(g_timer_id, 0, &ts, 0);
  if (rc == -1) {
    timer_delete(g_timer_id);
    THROW_EXP(0, 0, "timer_settime() failed", NULL);
  }

  g_clock_started = 1;
}

////////////////////////////////////////////////////////////////

static void stop_clock(void)
{
  int rc;

  // Check if timer running
  if (!g_clock_started) {
    cout << "WARNING: Clock not started\n";
    return;
  }

  // Delete timer
  rc = timer_delete(g_timer_id);
  if (rc == -1) {
    THROW_EXP(0, 0, "timer_delete() failed", NULL);
  }

  g_clock_started = 0;
}

////////////////////////////////////////////////////////////////

static void print_menu(void)
{
  cout << "------------------------------------\n";
  cout << "------ LCD6100_CLOCK_DEMO MENU -----\n";
  cout << "------------------------------------\n";
  cout << "\n";
  cout << "  1. Get product info\n";
  cout << "  2. Start clock\n";
  cout << "  3. Stop clock\n";
  cout << "100. Exit\n\n";
}

////////////////////////////////////////////////////////////////

static void clock_demo_menu(void)
{  
  int value;

  do {
    print_menu();
    
    cout << "Enter choice : ";
    scanf("%d",&value);
    
    switch(value) {
    case 1:
      get_prod_info();
      break;
    case 2:
      start_clock();
      break;
    case 3:
      stop_clock();
      break;
    case 100: // Exit
      break;
    default:
      cout << "Illegal choice!\n";
    }
  } while (value != 100);

  return;
}

////////////////////////////////////////////////////////////////

static void lcd6100_clock_demo(void)
{
  pthread_t ctrl_thread;
  int rc;

  try {
    g_lcd->initialize(); // Initialize LCD
    
    // Create controller thread
    rc = pthread_create(&ctrl_thread,
			NULL,
			ctrl_thread_func,
			NULL);
    if (rc) {
      THROW_EXP(0, 0, "pthread_create(CTRL), failed", NULL);
    }
  }
  catch (...) {
    g_lcd->finalize();
    throw;
  }

  // Controller thread is now created ...

  try {
    // Wait for controller thread to start
    if (sem_wait(&g_ctrl_sem)) {
      THROW_EXP(0, 0, "sem_wait(CTRL), failed", NULL);
    }
    clock_demo_menu();               // Run menu and handle user input
    g_run_demo = 0;                  // Signal controller thread to quit    
    pthread_join(ctrl_thread, NULL); // Wait for thread to terminate    
    g_lcd->finalize();               // Finalize LCD
  }
  catch (...) {
    g_run_demo = 0;                  // Signal controller thread to quit
    pthread_join(ctrl_thread, NULL); // Wait for thread to terminate
    g_lcd->finalize();               // Finalize LCD
    throw;
  }
}

////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  try {
    g_lcd = new lcd(DEMO_REVISION,
		    LCD_IFACE,
		    LCD_HW_RES_PIN,
		    LCD_CE,
		    LCD_SPEED);

    sem_init(&g_ctrl_sem, 0, 0); // Initial value is busy

    lcd6100_clock_demo();

    sem_destroy(&g_ctrl_sem);

    delete g_lcd;
  }
  catch (excep &exp) {
    cout << exp.what();
    sem_destroy(&g_ctrl_sem);
    delete g_lcd;
  }
  catch (...) {
    cout << "Unknown exception in main\n";
    sem_destroy(&g_ctrl_sem);
    delete g_lcd;
  }
  
  return 0;
}
