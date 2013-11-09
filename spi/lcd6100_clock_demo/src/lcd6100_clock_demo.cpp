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
#include <exception>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <iostream>

#include "lcd6100.h"
#include "lcd.h"
#include "delay.h"
#include "excep.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define DEMO_REVISION  "R1A01"

#define LCD_IFACE  LCD6100_IFACE_BITBANG
#define LCD_CE     LCD6100_CE_0
#define LCD_SPEED  0

/////////////////////////////////////////////////////////////////////////////
//               Definitions of types
/////////////////////////////////////////////////////////////////////////////

// Data periodically updated by controller thread
typedef struct {
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
} LCD_DIGITAL_TIME;

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

// Thread function prototypes
static void timer_thread_func(union sigval sv);
static void *ctrl_thread_func(void *ptr);

// Non-thread function prototypes
static void lcd6100_clock_demo_terminate(void);
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

static LCD_DIGITAL_TIME g_lcd_digital_time = {0, 0, 0};
static bool g_update = true;
static pthread_mutex_t g_update_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool g_timer_thread_error = false;

////////////////////////////////////////////////////////////////

static void timer_thread_func(union sigval sv)
{
  // Don't run if any prevoius error in this thread
  if (g_timer_thread_error) {
    return;
  }

  // If controller got mutex here, we are in trouble and late!
  if (pthread_mutex_trylock(&g_update_mutex) == EBUSY) {
    cout << "WARNING: Timer thread got no mutex, are we late?\n";
    return;
  }

  try {
    g_lcd->draw_digital_time(); // Draw LCD digital time    
    g_lcd->draw_analog_time();  // Draw LCD analog time
    
    // Signal controller thread ok to update new data
    g_update = true;
  } 
  catch (excep &exp) {
    cout << exp.what();
    g_timer_thread_error = true;
  }
  catch (...) {
    cout << "Unknown exception in timer thread\n";
    g_timer_thread_error = true;
  }

  pthread_mutex_unlock(&g_update_mutex);
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
      if (delay(0.10) != DELAY_SUCCESS) {
	THROW_EXP(0, 0, "controller thread, delay() failed", NULL);
      }
      
      // No need to update if clock is not running
      if (!g_clock_started) {
	continue;
      }
      
      pthread_mutex_lock(&g_update_mutex);
      
      // Only update if timer thread has drawn old data
      // This means that one second has passed
      // Prepare (update) data for next drawing
      if (!g_update) {
	pthread_mutex_unlock(&g_update_mutex);
	continue;
      }
      
      // Update digital time
      if (++g_lcd_digital_time.sec == 60) {
	g_lcd_digital_time.sec = 0;
	if (++g_lcd_digital_time.min == 60) {
	  g_lcd_digital_time.min = 0;
	  if (++g_lcd_digital_time.hour == 24) {
	    g_lcd_digital_time.hour = 0;
	  }
	}      
      }
      g_lcd->update_digital_time(g_lcd_digital_time.hour,
				 g_lcd_digital_time.min,
				 g_lcd_digital_time.sec);
      // Update analog time
      // JOE: TBD
      
      g_update = false; // No need to update until
                        // timer thread has drawn this data
      
      pthread_mutex_unlock(&g_update_mutex);
    }    
  } 
  catch (excep &exp) {
    cout << exp.what();
    pthread_mutex_unlock(&g_update_mutex);
  }
  catch (...) {
    cout << "Unknown exception in controller thread\n";
    pthread_mutex_unlock(&g_update_mutex);
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
  g_lcd->update_digital_time(0, 0, 0);
  g_lcd->draw_digital_time();

  // Reset analog clock on LCD
  // JOE: TBD

  // Signal controller thread ok to update new data
  pthread_mutex_lock(&g_update_mutex);
  g_lcd_digital_time.hour = 0;
  g_lcd_digital_time.min  = 0;
  g_lcd_digital_time.sec  = 0;
  g_update = true;
  pthread_mutex_unlock(&g_update_mutex);

  // Set the sigevent structure to cause the signal
  // to be delivered by creating a new thread.
  bzero(&se, sizeof(se));
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &g_timer_id;
  se.sigev_notify_function = timer_thread_func;
  se.sigev_notify_attributes = NULL;

  // Specify a repeating timer that fires each 1 seconds.
  // First expiration is after 1 seconds.
  ts.it_value.tv_sec = 1;
  ts.it_value.tv_nsec = 0;
  ts.it_interval.tv_sec = 1;
  ts.it_interval.tv_nsec = 0;

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
  cout << "  1. Start clock\n";
  cout << "  2. Stop clock\n";
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
      start_clock();
      break;
    case 2:
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
