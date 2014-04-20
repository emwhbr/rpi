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

#ifndef __SHELL_CMD_H__
#define __SHELL_CMD_H__

#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define SHELL_CMD_SUCCESS       0
#define SHELL_CMD_NOT_FOUND    -1
#define SHELL_CMD_FORK_FAILED  -2
#define SHELL_CMD_WAIT_FAILED  -3

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class shell_cmd {

 public:
  shell_cmd(void);
  ~shell_cmd(void);

  long check_command(const string cmd,
		     bool &found);
  
  // No output on stdout
  long execute(const string cmd,
	       string &output);

  // Output on stdout
  long execute(const string cmd,
	       int &exit_status);

 private:
  void init_members(void);

  string get_cmd_no_args(const string cmd);

  long execute_shell_cmd(const string cmd,
			 int &exit_status);
};

#endif // __SHELL_CMD_H__
