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

#include "eprom24x.h"
#include "eprom24x_timer.h"
#include "eprom24x_exception.h"

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

eprom24x_timer::eprom24x_timer(void)
{
  m_running_time  = 0.0;
  m_timer_running = false;

  reset();
}

////////////////////////////////////////////////////////////////

eprom24x_timer::~eprom24x_timer(void)
{
}

////////////////////////////////////////////////////////////////

void eprom24x_timer::reset(void)
{
  // Save start time
  if ( clock_gettime(CLK_ID, &m_start_time) ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_CLOCK_OPERATION_FAILED,
	      "save start time failed");
  }

  // Save last-start-time
  if ( clock_gettime(CLK_ID, &m_last_start_time) ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_CLOCK_OPERATION_FAILED,
	      "save last-start-time failed");
  }

  m_running_time  = 0.0;
  m_timer_running = true;
}

////////////////////////////////////////////////////////////////

void eprom24x_timer::pause(void)
{
  struct timespec now_time;

  // Check if already paused
  if (!m_timer_running) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_OPERATION_NOT_ALLOWED,
	      "timer already paused");
  }

  // Get now-time
  if ( clock_gettime(CLK_ID, &now_time) ) {
     THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_CLOCK_OPERATION_FAILED,
	       "get now-time failed");
  }

  // Calculate elapsed time since resume
  m_running_time += get_time_diff(m_last_start_time, now_time);

  m_timer_running = false;
}

////////////////////////////////////////////////////////////////

void eprom24x_timer::resume(void)
{
  // Check if already running
  if (m_timer_running) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_OPERATION_NOT_ALLOWED,
	      "timer already running");
  }

  // Save last start time
  if ( clock_gettime(CLK_ID, &m_last_start_time) ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_CLOCK_OPERATION_FAILED,
	      "save last start time failed");
  }

  m_timer_running = true;
}

////////////////////////////////////////////////////////////////

double eprom24x_timer::get_elapsed_time(void)
{
  struct timespec now_time;

  // Get now-time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_CLOCK_OPERATION_FAILED,
	      "get now-time failed");
  }

  // Calculate elapsed time since reset
  return get_time_diff(m_start_time, now_time);
}

////////////////////////////////////////////////////////////////

double eprom24x_timer::get_running_time(void)
{
  struct timespec now_time;

  // Check if paused
  if (!m_timer_running) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_OPERATION_NOT_ALLOWED,
	      "timer paused");
  }

  // Timer is still running ...

  // Get now-time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_CLOCK_OPERATION_FAILED,
	      "get now-time failed");
  }

  // Calculate elapsed time since resume
  return m_running_time + get_time_diff(m_last_start_time, now_time);
}

////////////////////////////////////////////////////////////////

bool eprom24x_timer::is_running(void)
{
  return m_timer_running;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

double eprom24x_timer::get_time_diff(timespec start, timespec end)
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
