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

#include "eprom24x_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
*
* Name eprom24x_exception
*
* Description Class constructor.
*
* Parameters source       IN  Classification of the error
*            code         IN  The error code
*            info_format  IN  Information (formatted) that describes the error
*            ...          IN  Variable number of arguments for 'info_format'
*
* Error handling None
*
****************************************************************************/
eprom24x_exception::eprom24x_exception(const char *file,
				       int line,
				       const char *pretty_function,
				       EPROM24x_ERROR_SOURCE source,
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

/****************************************************************************
*
* Name ~eprom24x_exception
*
* Description Class destructor.
*
* Parameters None
*
* Error handling None
*
****************************************************************************/
eprom24x_exception::~eprom24x_exception(void) throw()
{
}

/****************************************************************************
*
* Name get_function
*
* Description Returns the function for the exception.
*
* Parameters None
*
* Error handling None
*
****************************************************************************/
string eprom24x_exception::get_function(void)
{
  return get_class_method(m_pretty_function);
}

/****************************************************************************
*
* Name get_stack_frames
*
* Description Returns the stack trace for the exception.
*
* Parameters TBD
*
* Error handling None
*
****************************************************************************/
void eprom24x_exception::get_stack_frames(STACK_FRAMES &frames)
{
  unsigned i = 0;

  bzero(&frames, sizeof(frames));

  while ( m_stack_frames[i] ) {
    frames.frames[i] = (uint32_t)m_stack_frames[i];
    i++;
  }

  frames.active_frames = i;
}

/****************************************************************************
*
* Name what
*
* Description Overrides exception::what.
*
* Parameters None
*
* Error handling None
*
****************************************************************************/
const char* eprom24x_exception::what() const throw()
{
  ostringstream oss_msg;

  // Writes the output into the in-memory ostringstream class object,
  // converting values into string representation
  oss_msg << "------------ EPROM24x-EXCEPTION : BEGIN ------------\n";
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

  oss_msg << "------------ EPROM24x-EXCEPTION : END ------------";

  // Return the const char* representation
  return oss_msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

string eprom24x_exception::get_class_method(const string pretty_function)
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

