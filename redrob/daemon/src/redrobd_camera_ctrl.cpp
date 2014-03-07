// ************************************************************************
// *                                                                      *
// * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include "redrobd_camera_ctrl.h"
#include "redrobd_log.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define RPI_STREAM_SCRIPT "/proj/redrob/video_stream/rpi_video_stream.sh"

#define RPI_STREAM_SCRIPT_EXIT_OK    0
#define RPI_STREAM_SCRIPT_EXIT_FAIL  1

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_camera_ctrl::redrobd_camera_ctrl(void)
{
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_camera_ctrl::~redrobd_camera_ctrl(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_camera_ctrl::initialize(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_camera_ctrl::finalize(void)
{
  if (m_current_state == CC_STATE_ACTIVE) {
    stop_stream();
    m_current_state = CC_STATE_DEACTIVE;
  }
}

////////////////////////////////////////////////////////////////

void redrobd_camera_ctrl::command(uint16_t code)
{
  // Check of allowed codes
  if ( !check_camera_code(code) ) {
    return;
  }

  CC_STATE next_state = CC_STATE_INIT;

  switch (m_current_state) {
    /////////////////////////////////////////////////////
  case CC_STATE_INIT:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_CC_NONE:
      next_state = CC_STATE_INIT;
      break;
    case REDROBD_CC_STOP_STREAM:
      next_state = CC_STATE_INIT;
      break;
    case REDROBD_CC_START_STREAM:
      start_stream();
      next_state = CC_STATE_ACTIVE;
    }
    break;
    /////////////////////////////////////////////////////
  case CC_STATE_ACTIVE:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_CC_NONE:
      next_state = CC_STATE_ACTIVE;
      break;
    case REDROBD_CC_STOP_STREAM:
      stop_stream();
      next_state = CC_STATE_DEACTIVE;
      break;
    case REDROBD_CC_START_STREAM:
      next_state = CC_STATE_ACTIVE;
    }
    break;
    /////////////////////////////////////////////////////
  case CC_STATE_DEACTIVE:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_CC_NONE:
      next_state = CC_STATE_DEACTIVE;
      break;
    case REDROBD_CC_STOP_STREAM:
      next_state = CC_STATE_DEACTIVE;
      break;
    case REDROBD_CC_START_STREAM:
      start_stream();
      next_state = CC_STATE_ACTIVE;
    }
    break;
  }

  m_current_state = next_state;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_camera_ctrl::init_members(void)
{
  m_current_state = CC_STATE_INIT;
}

////////////////////////////////////////////////////////////////

bool redrobd_camera_ctrl::check_camera_code(uint16_t code)
{
  bool camera_code_ok = false;

  switch (code) {
  case REDROBD_CC_NONE:
  case REDROBD_CC_STOP_STREAM:
  case REDROBD_CC_START_STREAM:
    camera_code_ok = true;
    break;
  default:
    // All other camera codes are ignored for now
    ostringstream oss_msg;
    oss_msg << "Camera control : Got undefined camera code = 0x"
	    << hex << setw(4) << setfill('0') << (unsigned)code;
    redrobd_log_writeln(oss_msg.str());

    camera_code_ok = false;
  }

  return camera_code_ok;
}

////////////////////////////////////////////////////////////////

void redrobd_camera_ctrl::stop_stream(void)
{
  const string stop_stream_cmd = string(RPI_STREAM_SCRIPT) + string(" shutdown");

  // Execute stream script (shutdown stream)
  int rc = system(stop_stream_cmd.c_str());
  if (rc == -1) {
    redrobd_log_writeln("Camera control: Stop stream [FORK FAILED]");
  }
  else {
    // Check script exit status
    if (  WEXITSTATUS(rc) == RPI_STREAM_SCRIPT_EXIT_OK ) {
      redrobd_log_writeln("Camera control: Stop stream [OK]");
    }
    else if ( WEXITSTATUS(rc) == RPI_STREAM_SCRIPT_EXIT_FAIL ) {
      redrobd_log_writeln("Camera control: Stop stream [FAIL]");
    }
    else {
      redrobd_log_writeln("Camera control: Stop stream [UNKNOWN]");
    }
  }
}

////////////////////////////////////////////////////////////////

void redrobd_camera_ctrl::start_stream(void)
{
  const string start_stream_cmd = string(RPI_STREAM_SCRIPT) + string(" start");

  // Execute stream script (start stream)
  int rc = system(start_stream_cmd.c_str());
  if (rc == -1) {
    redrobd_log_writeln("Camera control: Start stream [FORK FAILED]");
  }
  else {
    // Check script exit status
    if (  WEXITSTATUS(rc) == RPI_STREAM_SCRIPT_EXIT_OK ) {
      redrobd_log_writeln("Camera control: Start stream [OK]");
    }
    else if ( WEXITSTATUS(rc) == RPI_STREAM_SCRIPT_EXIT_FAIL ) {
      redrobd_log_writeln("Camera control: Start stream [FAIL]");
    }
    else {
      redrobd_log_writeln("Camera control: Start stream [UNKNOWN]");
    }
  }
}
