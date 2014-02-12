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

#ifndef __TIMER_H__
#define __TIMER_H__

#include <time.h>

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define TIMER_SUCCESS   0
#define TIMER_FAILURE  -1

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class timer {

 public:
  timer(void);
  ~timer(void);
  
  long reset(void);
  long pause(void);
  long resume(void);

  double get_elapsed_time(void); // Total time (paused + running)
  double get_running_time(void); // Only time when NOT paused

  bool is_running(void);

 private:
  bool	  m_timer_running;
  double  m_running_time;

  struct timespec m_start_time;
  struct timespec m_last_start_time;

  double get_time_diff(timespec start, timespec end);
};

#endif // __TIMER_H__
