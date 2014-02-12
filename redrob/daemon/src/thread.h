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

#ifndef __THREAD_H__
#define __THREAD_H__

#include <string>
#include <semaphore.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define THREAD_SUCCESS           0
#define THREAD_WRONG_STATE      -1
#define THREAD_PTHREAD_ERROR    -2
#define THREAD_SEMAPHORE_ERROR  -3
#define THREAD_MUTEX_ERROR      -4
#define THREAD_TIME_ERROR       -5
#define THREAD_INTERNAL_ERROR   -6 // Used by derived class

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

typedef enum {THREAD_STATE_NOT_STARTED = 10,
	      THREAD_STATE_STARTED     = 20,
	      THREAD_STATE_SETUP_DONE  = 30,
	      THREAD_STATE_EXECUTING   = 40,
	      THREAD_STATE_DONE        = 50} THREAD_STATE;

// Bitmask values for decoding thread status
#define THREAD_STATUS_OK              0
#define THREAD_STATUS_SETUP_FAILED    1
#define THREAD_STATUS_EXECUTE_FAILED  2
#define THREAD_STATUS_CLEANUP_FAILED  4
#define THREAD_STATUS_DONE_FAILED     8

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class thread {

 public:
  thread(string thread_name);
  virtual ~thread(void);

  long start(void *p_arg);  // Create and start thread
  long release(void);       // Release thread (execute)
  long stop(void);          // Order thread to stop executing
  long wait(void);          // Wait for thread to complete (Pthread-join)
  
  long wait_timed(double timeout_in_sec); // Wait for thread to complete
                                          // using a timeoute (Pthread-timed-join)

  THREAD_STATE get_state(void) {return m_state;} // Thread state
  unsigned get_status(void) {return m_status;}   // Thread status

  string get_name(void);

  pid_t get_tid(void);
  pid_t get_pid(void);

  unsigned get_exe_cnt(void);

 protected:  
  void run(void *p_arg);
  
  void *arg(void) const {return m_arg;}
  void arg(void *arg) {m_arg = arg;}

  static void *entry_point(void *p_this);

  void update_exe_cnt(void);
  bool is_stopped(void);
  
  virtual long setup(void) = 0;        // Pure virtual function
  virtual long execute(void *arg) = 0; // Pure virtual function
  virtual long cleanup(void) = 0;      // Pure virtual function
    
 private:
  string m_thread_name;

  THREAD_STATE m_state;
  unsigned     m_status;

  void *m_arg;

  pthread_t m_thread;  // Thread handle
  pid_t     m_tid;     // Thread ID
  pid_t     m_pid;     // Thread PID

  // Handles/signals 'thread done'
  pthread_cond_t     m_cond_thread_done;
  pthread_condattr_t m_condattr_thread_done;
  pthread_mutex_t    m_mutex_thread_done;
  
  unsigned m_exe_cnt;  // Thread execution counter
  bool     m_stop;     // Thread has been ordered to stop

  sem_t m_sem_release; // Released when thread shall execute

  void init_members(void);
};

#endif // __THREAD_H__
