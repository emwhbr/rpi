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

#include <stddef.h>

#include "timer.h"

// Use best available clock identifer
#if defined CLOCK_MONOTONIC_RAW
#define CLK_ID CLOCK_MONOTONIC_RAW
#elif defined CLOCK_MONOTONIC
#warning CLOCK_MONOTONIC_RAW not defined, using CLOCK_MONOTONIC
#define CLK_ID CLOCK_MONOTONIC
#elif defined CLOCK_REALTIME
#warning CLOCK_MONOTONIC_RAW and CLOCK_MONOTONIC not defined, using CLOCK_REALTIME
#define CLK_ID CLOCK_REALTIME
#else
#error CLOCK_xxx not defined, abort
#endif

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

timer::timer(void)
{
  m_running_time  = 0.0;
  m_timer_running = false;

  reset();
}

////////////////////////////////////////////////////////////////

timer::~timer(void)
{
}

////////////////////////////////////////////////////////////////

long timer::reset(void)
{
  // Save start time
  if ( clock_gettime(CLK_ID, &m_start_time) ) {
    return TIMER_FAILURE;
  }

  // Save last-start-time
  if ( clock_gettime(CLK_ID, &m_last_start_time) ) {
    return TIMER_FAILURE;
  }

  m_running_time  = 0.0;
  m_timer_running = true;

  return TIMER_SUCCESS;
}

////////////////////////////////////////////////////////////////

long timer::pause(void)
{
  struct timespec now_time;

  // Check if already paused
  if (!m_timer_running) {
    return TIMER_FAILURE;
  }

  // Get now-time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    return TIMER_FAILURE;
  }

  // Calculate elapsed time since resume
  m_running_time += get_time_diff(m_last_start_time, now_time);

  m_timer_running = false;

  return TIMER_SUCCESS;
}

////////////////////////////////////////////////////////////////

long timer::resume(void)
{
  // Check if already running
  if (m_timer_running) {
    return TIMER_FAILURE;
  }

  // Save last start time
  if ( clock_gettime(CLK_ID, &m_last_start_time) ) {
    return TIMER_FAILURE;
  }

  m_timer_running = true;

  return TIMER_SUCCESS;
}

////////////////////////////////////////////////////////////////

double timer::get_elapsed_time(void)
{
  struct timespec now_time;

  // Get now time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    return -1.0;
  }

  // Calculate elapsed time since reset
  return get_time_diff(m_start_time, now_time);
}

////////////////////////////////////////////////////////////////

double timer::get_running_time(void)
{
  struct timespec now_time;

  // Check if paused
  if (!m_timer_running) {
      return m_running_time;
  }

  // Timer is still running ...

  // Get now time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    return -1.0;
  }

  // Calculate elapsed time since resume
  return m_running_time + get_time_diff(m_last_start_time, now_time);
}

////////////////////////////////////////////////////////////////

bool timer::is_running(void)
{
  return m_timer_running;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

double timer::get_time_diff(timespec start, timespec end)
{
  timespec temp;

  // This function assumes end >= start, use with care

  if ( (end.tv_nsec - start.tv_nsec) < 0 ) {
    // Borrow one second
    temp.tv_sec  = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000L + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec  = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }

  return (double)temp.tv_sec +
         (double)(temp.tv_nsec) / 1000000000.0;
}
