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
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

#include "daemon_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static long acquire_lock_file(const char *lock_file,
			      int *fd_lock_file)
{
  long rc = DAEMON_FAILURE;
  int fd;

  *fd_lock_file = DAEMON_BAD_FD_LOCK_FILE; // No file descriptor yet
  
  fd = open(lock_file, O_RDWR | O_CLOEXEC | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd != -1) {
  
    struct flock fl;
    fl.l_type   = F_WRLCK;  // Place a write lock
    fl.l_whence = SEEK_SET; // on the entire file
    fl.l_start  = 0;
    fl.l_len    = 0;
    fl.l_pid    = getpid();
 
    if (fcntl(fd, F_SETLK, &fl) != -1) {
      // We have locked file, truncate to zero bytes
      if (ftruncate(fd, 0) != -1) {
	// Write PID to file
	char pid_text[20];
	snprintf(pid_text, sizeof(pid_text), "%ld\n", (long)getpid());
	if (write(fd, pid_text, strlen(pid_text)) == (int)strlen(pid_text)) {
	  rc = DAEMON_SUCCESS; // We created the PID lock file,
	                       // truncated, and write the PID to it.
	  *fd_lock_file = fd;  // Return descriptor to caller
	}
      }
    }    
  }

  if ( (rc != DAEMON_SUCCESS) && (fd != -1) ) {
    close(fd);
    syslog_error("Unable to acquire lock file '%s', code=%d (%s)",
		 lock_file, errno, strerror(errno));
  }

  return rc;
}

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void syslog_open(const char *ident)
{
  openlog(ident, LOG_PID, LOG_USER);
}

////////////////////////////////////////////////////////////////

void syslog_info(const char *format, ...)
{
  // Retrieve any additional arguments for the format string
  char info_buffer[512];
  va_list info_args;
  va_start(info_args, format);
  vsprintf(info_buffer, format, info_args);
  va_end(info_args);

  // Log message
  syslog(LOG_NOTICE, info_buffer);
}

////////////////////////////////////////////////////////////////

void syslog_error(const char *format, ...)
{
  // Retrieve any additional arguments for the format string
  char info_buffer[512];
  va_list info_args;
  va_start(info_args, format);
  vsprintf(info_buffer, format, info_args);
  va_end(info_args);

  // Log message
  syslog(LOG_ERR, info_buffer);
}

////////////////////////////////////////////////////////////////

void syslog_close(void)
{
  closelog();  
}

////////////////////////////////////////////////////////////////

long define_signal_handler(int sig, void (*handler)(int))
{
  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags   = SA_RESTART;
  sa.sa_handler = handler;

  if (sigaction(sig, &sa, NULL) == -1) {
    syslog_error("sigaction failed for signal(%d), code=%d (%s)",
		 sig, errno, strerror(errno));
    return DAEMON_FAILURE;
  }

  return DAEMON_SUCCESS;
}

////////////////////////////////////////////////////////////////

long send_signal_self(int sig)
{
  // Send signal to calling process
  if (kill(getpid(), sig) == -1) {
    syslog_error("kill failed for signal(%d), code=%d (%s)",
		 sig, errno, strerror(errno));
    return DAEMON_FAILURE;
  }

  return DAEMON_SUCCESS;
}

////////////////////////////////////////////////////////////////

long become_daemon(const char *run_as_user,
		   const char *work_dir,
		   const char *lock_file,
		   int *fd_lock_file)
{
  pid_t pid;
  int fd;

  *fd_lock_file = DAEMON_BAD_FD_LOCK_FILE;

  // Switch to specified user
  if (run_as_user) {
    struct passwd *pw_info = getpwnam(run_as_user);
    if (pw_info) {
      if (setuid(pw_info->pw_uid) == -1) {
	syslog_error("setuid failed for %s, code=%d (%s)",
		     run_as_user, errno, strerror(errno));
	return DAEMON_FAILURE;
      }
    }
    else {
      syslog_error("getpwnam failed for %s", run_as_user);
      return DAEMON_FAILURE;
    }
  }

  // Step 1 : Create a child process
  pid = fork();
  switch (pid) {
  case -1 :
    syslog_error("fork(1) failed, code=%d (%s)", errno, strerror(errno));
    return DAEMON_FAILURE;
  case 0 :
    break;               // Child falls through
  default :
    exit(EXIT_SUCCESS);  // Parent terminates OK
  }

  // Step 2 : Become leader of new session
  if (setsid() == -1) {
    syslog_error("setsid failed, code=%d (%s)", errno, strerror(errno));
    return DAEMON_FAILURE;
  }

  // Step 3 : Ensure no controlling terminal
  pid = fork();
  switch (pid) {
  case -1 :
    syslog_error("fork(2) failed, code=%d (%s)", errno, strerror(errno));
    return DAEMON_FAILURE;
  case 0 :
    break;               // Grand-child falls through
  default :
    exit(EXIT_SUCCESS);  // Child terminates OK
  }

  // This process (grand-child) will be the actual daemon

  // Step 4 : Clear file mode creation mask
  umask(0);

  // Step 5 : Change current working directory
  if (chdir(work_dir) == -1) {
    syslog_error("chdir to '%s' failed, code=%d (%s)",
		 work_dir, errno, strerror(errno));
    return DAEMON_FAILURE;
  }

  // Step 6: Close standard file descriptors
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Step 7 : Redirect standard file descriptors to /dev/null
  fd = open("/dev/null", O_RDWR);
  if (fd != STDIN_FILENO) {
    syslog_error("Redirect STDIN failed, code=%d (%s)",
		 errno, strerror(errno));
    return DAEMON_FAILURE;
  }
  if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
    syslog_error("Redirect STDOUT failed, code=%d (%s)",
		 errno, strerror(errno));
    return DAEMON_FAILURE;
  }
  if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
    syslog_error("Redirect STDERR failed, code=%d (%s)",
		 errno, strerror(errno));
    return DAEMON_FAILURE;
  }

  // Create lock file to prevent more than one instance
  // of the daemon is being executed
  if (lock_file) {
    if (acquire_lock_file(lock_file,
			  fd_lock_file) != DAEMON_SUCCESS) {
      return DAEMON_FAILURE;
    }
  }

  return DAEMON_SUCCESS;
}
