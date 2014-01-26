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

#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <exception>

#include "redrobd.h"
#include "daemon_utility.h"
#include "delay.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Function prototypes
/////////////////////////////////////////////////////////////////////////////

static void daemon_terminate(void);
static void daemon_parse_command_line(int argc, char *argv[]);
static void daemon_signal_handler(int sig);
static void daemon_exit_on_error(int fd_lock_file);
static void daemon_report_prod_info(void);
static int  daemon_get_config(REDROBD_CONFIG *config);
static int  daemon_check_status(void);

/////////////////////////////////////////////////////////////////////////////
//               Global variables
/////////////////////////////////////////////////////////////////////////////

class the_terminate_handler {
public:
  the_terminate_handler() {
    set_terminate( daemon_terminate );
  }
};
// Install terminate function (in case of emergency)
// Install as soon as possible, before main starts
static the_terminate_handler g_terminate_handler;

static volatile sig_atomic_t g_received_sig_restart   = 0;
static volatile sig_atomic_t g_received_sig_terminate = 0;

static REDROBD_CONFIG g_config;

static bool g_is_daemon;

static int g_do_shutdown = 1; // Enable/disable system shutdown

////////////////////////////////////////////////////////////////

static void daemon_terminate(void)
{
  // Only log this event, no further actions for now
  syslog_error("Unhandled exception, termination handler activated (shutdown=%s)",
	       (g_do_shutdown ? "true" : "false"));

  // Initiate system shutdown
  if (g_do_shutdown) {
    shutdown_system();
  }
 
  // The terminate function should not return
  abort();
}

////////////////////////////////////////////////////////////////

static void daemon_parse_command_line(int argc, char *argv[])
{
  int option_index = 0;
  int c;
  struct option long_options[] = {
    {"no_shutdown", no_argument, &g_do_shutdown, 0},
    {0, 0, 0, 0}
  };

  while (1) {
    c = getopt_long(argc, argv, "",
		    long_options, &option_index);
    
    // Detect the end of the options
    if (c == -1) {
      break;
    }
    
    switch (c) {
    case 0:
      // If option sets a flag, do nothing else now
      if (long_options[option_index].flag) {
	break;
      }
      break;
    default:
      break;
    }
  }
}

////////////////////////////////////////////////////////////////

static void daemon_signal_handler(int sig)
{
  switch (sig) {
  case SIG_RESTART_DAEMON:
    g_received_sig_restart = 1;
    break;
  case SIG_TERMINATE_DAEMON:
    g_received_sig_terminate = 1;
    break;
  default:
    ;
  }
}

////////////////////////////////////////////////////////////////

static void daemon_exit_on_error(int fd_lock_file)
{
  syslog_info("Terminated bad (shutdown=%s)",
	      (g_do_shutdown ? "true" : "false"));
  syslog_close();

  if (fd_lock_file != DAEMON_BAD_FD_LOCK_FILE) {
    close(fd_lock_file);
    unlink(g_config.lock_file);
  }

  // Initiate system shutdown
  if (g_do_shutdown) {
    shutdown_system();
  }

  exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////

static void daemon_report_prod_info(void)
{
  REDROBD_PROD_INFO prod_info;
  redrobd_get_prod_info(&prod_info);

  syslog_info("%s - %s",
	      prod_info.prod_num,
	      prod_info.rstate);
}

////////////////////////////////////////////////////////////////

static int daemon_get_config(REDROBD_CONFIG *config)
{
  if (redrobd_get_config(config) != REDROBD_SUCCESS) {
    REDROBD_STATUS status;
    redrobd_get_last_error(&status);
    syslog_error("Configuration file error, source:%d, code:%ld\n",
		 status.error_source, status.error_code);
    return 0;
  }
  
  // Note!!
  // To avoid problems with syslog multiline-messages (embedded '\n')
  // we use '\\n' as a newline separator.
  // This message can be decoded as a multiline message by examine
  // the messages log using sed-command:
  // tail -f /var/log/messages.log | sed 's/\\n/\n/g'

  ostringstream oss_msg;
  oss_msg << "Configuration:" << "\\n";
  oss_msg << "\tdaemonize :" << config->daemonize << "\\n";
  oss_msg << "\tuser      :" << config->user << "\\n";
  oss_msg << "\twork_dir  :" << config->work_dir << "\\n";
  oss_msg << "\tlock_file :" << config->lock_file  << "\\n";
  oss_msg << "\tlog_file  :" << config->log_file  << "\\n";
  oss_msg << "\tlog_stdout:" << config->log_stdout  << "\\n";
  oss_msg << "\tsup_freq  :" << config->supervision_freq << "\\n";
  oss_msg << "\tctrl_freq :" << config->ctrl_thread_freq << "\n";
  oss_msg << "\tverbose   :" << config->verbose << "\n";

  // Print all info
  syslog_info(oss_msg.str().c_str());

  return 1;
}

////////////////////////////////////////////////////////////////

static int daemon_check_status(void)
{
  REDROBD_STATUS status;

  // Check for errors in daemon
  if (redrobd_get_last_error(&status) == REDROBD_SUCCESS) {
    if (status.error_code != REDROBD_NO_ERROR) {
      syslog_error("Daemon status not OK, source:%d, code:%ld\n",
		   status.error_source, status.error_code);
      return 0;
    }
  }
  else {
    syslog_error("Can't get daemon status");
    return 0;
  }

  // Force a check of daemon status, ignore result now
  // Any errors will be detected next time this function is called
  redrobd_check_run_status();

  return 1;
}

////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  long rc;
  int fd_lock_file = DAEMON_BAD_FD_LOCK_FILE;

  // Parse command line arguments
  daemon_parse_command_line(argc, argv);

  // Initialize handling of messages sent to system logger
  syslog_open(REDROBD_NAME);

  // Report product number and the RState
  daemon_report_prod_info();

  // Read configuration file
  if (!daemon_get_config(&g_config)) {
    daemon_exit_on_error(fd_lock_file);
  }

  // Define handler for daemon restart.
  // The daemon will reread its configuration file and
  // reopen its log file
  rc = define_signal_handler(SIG_RESTART_DAEMON, daemon_signal_handler);
  if (rc != DAEMON_SUCCESS) {
    daemon_exit_on_error(fd_lock_file);
  }

  // Define handler for daemon terminate.  
  rc = define_signal_handler(SIG_TERMINATE_DAEMON, daemon_signal_handler);
  if (rc != DAEMON_SUCCESS) {
    daemon_exit_on_error(fd_lock_file);
  }
  
  // Go through the steps in becoming a daemon (or not)
  if (g_config.daemonize) {
    rc = become_daemon(g_config.user,
		       g_config.work_dir,
		       g_config.lock_file,		     
		       &fd_lock_file);
    if (rc != DAEMON_SUCCESS) {
      daemon_exit_on_error(fd_lock_file);
    }
    g_is_daemon = true;
  }
  else {
    g_is_daemon = false;
  }
  
  // We are now running as a daemon (or not)
  syslog_info("Started");

  // Initialize daemon
  if (redrobd_initialize(g_config.log_file,
			 (!g_is_daemon) && g_config.log_stdout,
			 g_config.ctrl_thread_freq,
			 g_config.verbose) != REDROBD_SUCCESS) {
    daemon_exit_on_error(fd_lock_file);
  }

  // Daemon main supervision and control loop
  for (;;) {
    // Check if time to restart
    if (g_received_sig_restart) {
      syslog_info("Got signal to restart");
      g_received_sig_restart = 0;
      // Finalize
      if (redrobd_finalize() != REDROBD_SUCCESS) {
	daemon_exit_on_error(fd_lock_file);
      }
      // Read configuration file
      if (!daemon_get_config(&g_config)) {
	daemon_exit_on_error(fd_lock_file);
      }
      // Initialize
      if (redrobd_initialize(g_config.log_file,
			     (!g_is_daemon) && g_config.log_stdout,
			     g_config.ctrl_thread_freq,
			     g_config.verbose) != REDROBD_SUCCESS) {
	daemon_exit_on_error(fd_lock_file);
      }
    }
    // Check if time to quit
    if (g_received_sig_terminate) {
      syslog_info("Got signal to terminate");
      g_received_sig_terminate = 0;
      if (redrobd_finalize() != REDROBD_SUCCESS) {
	daemon_exit_on_error(fd_lock_file);
      }
      break;
    }
    // Check daemon status
    if (!daemon_check_status()) {
      syslog_info("Daemon status not OK, terminating");
      redrobd_finalize();
      daemon_exit_on_error(fd_lock_file);
    }
    // Take it easy
    if ( delay(1.0/g_config.supervision_freq) != DELAY_SUCCESS ) {
      syslog_info("Error when delay, terminating");
      redrobd_finalize();
      daemon_exit_on_error(fd_lock_file);
    }
  }
  
  // Cleanup and exit
  syslog_info("Terminated ok (shutdown=%s)",
	      (g_do_shutdown ? "true" : "false"));
  syslog_close();
  
  if (fd_lock_file != DAEMON_BAD_FD_LOCK_FILE) {
    close(fd_lock_file);
    unlink(g_config.lock_file);
  }

  // Initiate system shutdown
  if (g_do_shutdown) {
    shutdown_system();
  }

  exit(EXIT_SUCCESS);
}
