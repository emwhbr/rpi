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

#ifndef __DAEMON_UTILITY_H__
#define __DAEMON_UTILITY_H__

#include <signal.h>

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define DAEMON_SUCCESS  0
#define DAEMON_FAILURE -1

#define DAEMON_BAD_FD_LOCK_FILE -1

// Signals
#define SIG_RESTART_DAEMON     SIGHUP
#define SIG_TERMINATE_DAEMON   SIGTERM

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern void syslog_open(const char *ident);
extern void syslog_error(const char *format, ...);
extern void syslog_info(const char *format, ...);
extern void syslog_close(void);

extern long define_signal_handler(int sig, void (*handler)(int));
extern long send_signal_self(int sig);

extern long become_daemon(const char *run_as_user,  // IN
			  const char *work_dir,     // IN
			  const char *lock_file,    // IN
			  int *fd_lock_file);       // OUT

#endif // __DAEMON_UTILITY_H__
