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
#include <stdint.h>
#include <execinfo.h>
#include <strings.h>
#include <sstream>
#include <iomanip>

#include "excep.h"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

excep::excep(void)
{
  m_file = "";
  m_line = 0;
  m_pretty_function = "";

  m_source = 0;
  m_code   = 0;
  m_info   = "";

  m_nr_frames = 0;
  *m_stack_frames = NULL;
}

////////////////////////////////////////////////////////////////

excep::excep(const char *file,
	     int line,
	     const char *pretty_function,
	     long source,
	     long code,
	     const char *info_format, ...)
{
  // Get list of void pointers, return addresses for each stack frame
  bzero(m_stack_frames, sizeof(void *) * MAX_NR_STACK_FRAMES);
  m_nr_frames = backtrace(m_stack_frames, MAX_NR_STACK_FRAMES);

  // Handle the standard predefined macros
  m_file = file;
  m_line = line;
  m_pretty_function = pretty_function;

  // Handle internal error info
  m_source = source;
  m_code   = code;

  // Retrieve any additional arguments for the format string
  char info_buffer[512];
  va_list info_args;
  va_start(info_args, info_format);
  vsprintf(info_buffer, info_format, info_args);
  va_end(info_args);

  m_info = info_buffer;
}

////////////////////////////////////////////////////////////////

excep::~excep(void) throw()
{
}

////////////////////////////////////////////////////////////////

string excep::get_function(void)
{
  return get_class_method(m_pretty_function);
}

////////////////////////////////////////////////////////////////

void excep::get_stack_frames(STACK_FRAMES &frames)
{
  bzero(&frames, sizeof(frames));

  for (int j=0; j < m_nr_frames; j++) {
    frames.frames[j] = (uint32_t)m_stack_frames[j];
  }

  frames.active_frames = m_nr_frames;
}

////////////////////////////////////////////////////////////////

const char* excep::what() const throw()
{
  ostringstream oss_msg;

  // Writes the output into the in-memory ostringstream class object,
  // converting values into string representation
  oss_msg << "------------ EXCEP : BEGIN ------------\n";
  oss_msg << "source:" << m_source << ", code:" << m_code << "\n"
	  << "info:" << m_info << "\n"
	  << "stack frames:" << m_nr_frames << "\n";

  // Write the stack trace
  unsigned i=0;
  char buffer[18];
  while ( m_stack_frames[i] ) {
    sprintf(buffer, "0x%08x", (uint32_t)m_stack_frames[i]);
    oss_msg << "frame:" << dec << setw(2) << setfill('0') << i++
	    << "  addr:" << buffer << "\n";
  }

  // Write info from predefined macros
  oss_msg << m_file 
	  << ":" << m_line
	  << ", " << m_pretty_function << "\n";
  oss_msg << "------------ EXCEP : END ------------\n";

  // Return the const char* representation
  return oss_msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

string excep::get_class_method(const string pretty_function)
{
  string class_method = pretty_function;

  // Strip the parameter list
  size_t index = class_method.find("(");
  if (index == string::npos) {
    return class_method;  // Degenerated case 
  }
  class_method.erase( index );

  // Strip the return type
  index = class_method.rfind(" ");
  if (index == string::npos) {
    return class_method;  // Degenerated case
  }
  class_method.erase(0, index + 1);

  return class_method; // The stripped name = class::method
}
