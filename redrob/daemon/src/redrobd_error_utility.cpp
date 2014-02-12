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
#include <string.h>
#include <errno.h>
#include <sstream>
#include <iomanip>

#include "redrobd_error_utility.h"
#include "redrobd.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

string redrobd_error_syslog_string(excep exp)
{
  // Get the stack trace
  STACK_FRAMES frames;
  exp.get_stack_frames(frames);

  ostringstream oss_msg;
  char buffer[18];

  // Note!!
  // To avoid problems with syslog multiline-messages (embedded '\n')
  // we use '\\n' as a newline separator.
  // This message can be decoded as a multiline message by examine
  // the error log using sed-command:
  // tail -f /var/log/errors.log | sed 's/\\n/\n/g'

  oss_msg << "\\n";
  oss_msg << "\tstack frames:" << (int) frames.active_frames << "\\n";

  for (unsigned i=0; i < frames.active_frames; i++) {
    sprintf(buffer, "0x%08x", frames.frames[i]);
    oss_msg << "\tframe:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\\n";
  }

  // Get info from predefined macros
  oss_msg << "\tViolator: " << exp.get_file() 
	  << ":" << exp.get_line()
	  << ", " << exp.get_function() << "\\n";

  // Get the internal info
  oss_msg << "\tSource: " << exp.get_source()
	  << ", Code: " << exp.get_code() << "\\n";

  oss_msg << "\tInfo: " << exp.get_info() << "\\n";

  // Source of error (last multi-line, terminate with '\n')
  switch (exp.get_source()) {
  case REDROBD_INTERNAL_ERROR:
    oss_msg << "\tREDROBD INTERNAL ERROR\n";
    break;
  case REDROBD_LINUX_ERROR:
    oss_msg << "\tREDROBD LINUX ERROR - errno:" 
	    << errno << " => " << strerror(errno) << "\n";
    break;
  }

  // Return the resulting string
  return oss_msg.str();
}
