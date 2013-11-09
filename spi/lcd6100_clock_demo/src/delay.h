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

#ifndef __DELAY_H__
#define __DELAY_H__

#include <time.h>

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define DELAY_SUCCESS   0
#define DELAY_FAILURE  -1

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern clockid_t get_clock_id(void);

extern long get_new_time(const struct timespec *old_time,
			 double diff_in_sec,
			 struct timespec *new_time);

extern long delay(double time_in_sec);

extern long delay_until(const struct timespec *the_time);

#endif // __DELAY_H__
