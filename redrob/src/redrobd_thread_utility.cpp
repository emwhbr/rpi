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

#include "redrobd_thread_utility.h"
#include "redrobd.h"
#include "excep.h"
#include "timer.h"
#include "delay.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_thread_initialize_cyclic(cyclic_thread *ct,
				      double ct_start_timeout,
				      double ct_execute_timeout)
{
  // Step 1: Start thread
  if ( ct->start(NULL) != THREAD_SUCCESS ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_OPERATION_FAILED,
	      "Error start thread %s",
	      ct->get_name().c_str());
  }

  // Step 2: Wait for thread to complete setup
  timer thread_timer;
  bool thread_timeout = true;
  if ( thread_timer.reset() != TIMER_SUCCESS ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
	      "Error resetting timeout timer(start) for thread %s",
	      ct->get_name().c_str());
  }
  while ( thread_timer.get_elapsed_time() < ct_start_timeout ) {
    if ( ct->get_state() == THREAD_STATE_SETUP_DONE ) {
      thread_timeout = false;
      break;
    }
    if ( delay(0.1) != DELAY_SUCCESS ) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
		"Delay operation failed(start), waiting for thread %s",
		ct->get_name().c_str());
    }
  }
  if ( thread_timeout ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIMEOUT_OCCURRED,
	      "Timeout waiting for start thread %s",
	      ct->get_name().c_str());
  }
  if ( ct->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_STATUS_NOT_OK,
	      "Thread %s status not OK, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }

  // Step 3: Release thread
  if ( ct->release() != THREAD_SUCCESS ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_OPERATION_FAILED,
	      "Error release thread %s",
	      ct->get_name().c_str());
  }

  // Step 4: Wait for thread to start executing
  thread_timeout = true;
  if ( thread_timer.reset() != TIMER_SUCCESS ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
	      "Error resetting timeout timer(execute) for thread %s",
	      ct->get_name().c_str());
  }
  while ( thread_timer.get_elapsed_time() < ct_execute_timeout ) {
    if ( ct->get_state() == THREAD_STATE_EXECUTING ) {
      thread_timeout = false;
      break;
    }
    if ( delay(0.1) != DELAY_SUCCESS ) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
		"Delay operation failed(execute), waiting for thread %s",
		ct->get_name().c_str());
    }
  }
  if ( thread_timeout ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIMEOUT_OCCURRED,
	      "Timeout waiting for execute thread %s",
	      ct->get_name().c_str());
  }
  if ( ct->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_STATUS_NOT_OK,
	      "Thread %s status not OK, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }
}

////////////////////////////////////////////////////////////////

void redrobd_thread_finalize_cyclic(cyclic_thread *ct,
				    double ct_stop_timeout)
{
  // Step 1: Stop thread
  if ( ct->stop() != THREAD_SUCCESS ) {    
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_OPERATION_FAILED,
	      "Error stop thread %s, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }
  
  // Step 2: Wait for thread to complete
  if ( ct->wait_timed(ct_stop_timeout) != THREAD_SUCCESS ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_OPERATION_FAILED,
	      "Error wait_timed thread %s, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }

  // Step 3: Check final status
  if ( ct->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_OPERATION_FAILED,
	      "Error finalize thread %s, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }
}

////////////////////////////////////////////////////////////////

void redrobd_thread_check_cyclic(cyclic_thread *ct)
{
  // Check state (executing)
  if ( ct->get_state() != THREAD_STATE_EXECUTING ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_STATE_NOT_OK,
	      "Thread %s not executing, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }

  // Check status (ok)
  if ( ct->get_status() != THREAD_STATUS_OK ) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_THREAD_STATUS_NOT_OK,
	      "Thread %s status not OK, status:0x%x, state:%u",
	      ct->get_name().c_str(),
	      ct->get_status(),
	      ct->get_state());
  }
}
