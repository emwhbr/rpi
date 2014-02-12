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

#include <time.h>
#include <errno.h>

#include "delay.h"

// Use best available clock identifer
#if defined CLOCK_MONOTONIC
#define CLK_ID CLOCK_MONOTONIC
#elif defined CLOCK_REALTIME
#warning CLOCK_MONOTONIC not defined, using CLOCK_REALTIME
#define CLK_ID CLOCK_REALTIME
#else
#error CLOCK_xxx not defined, abort
#endif

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define NSEC_PER_SEC  1000000000L

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static inline void tsnorm(struct timespec *ts)
{
   while (ts->tv_nsec >= NSEC_PER_SEC) {
      ts->tv_nsec -= NSEC_PER_SEC;
      ts->tv_sec++;
   }
}

////////////////////////////////////////////////////////////////

static inline long do_clock_nanosleep(const struct timespec *ts)
{
  int rc;

  // Do the nanosleep, until an absolute time in the future
  do {
    rc = clock_nanosleep(CLK_ID, TIMER_ABSTIME, ts, NULL);
  } while ( rc && (rc == EINTR) );

  if ( rc && (rc != EINTR) ) {
    return DELAY_FAILURE;
  }

  return DELAY_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

clockid_t get_clock_id(void)
{
  return CLK_ID;
}

////////////////////////////////////////////////////////////////

long get_new_time(const struct timespec *old_time,
		  double diff_in_sec,
		  struct timespec *new_time)
{
  long nr_sec;
  long nr_nsec;

  // Check arguments
  if ( (!old_time) || (!new_time) ) {
    return DELAY_FAILURE;
  }
  if ( diff_in_sec < 0.0 ) {
    return DELAY_FAILURE; // Negative deltas not supported
  }

  new_time->tv_sec  = old_time->tv_sec;
  new_time->tv_nsec = old_time->tv_nsec;

  // Calculate future time
  nr_sec  = (long) diff_in_sec;
  nr_nsec = (diff_in_sec - (double) nr_sec) * (double) NSEC_PER_SEC;

  new_time->tv_sec  += nr_sec;
  new_time->tv_nsec += nr_nsec;

  tsnorm(new_time);

  return DELAY_SUCCESS;
}

////////////////////////////////////////////////////////////////

long delay(double time_in_sec)
{
  long rc;
  struct timespec now_time;
  struct timespec future_time;

  // Get now time
  if ( clock_gettime(CLK_ID, &now_time) ) {
    return DELAY_FAILURE;
  }

  // Calculate time in the future
  rc = get_new_time(&now_time, time_in_sec, &future_time);
  if ( rc != DELAY_SUCCESS ) {
    return rc;
  }

  // Sleep until future time
  return do_clock_nanosleep(&future_time);
}

////////////////////////////////////////////////////////////////

long delay_until(const struct timespec *the_time)
{
  return do_clock_nanosleep(the_time);
}
