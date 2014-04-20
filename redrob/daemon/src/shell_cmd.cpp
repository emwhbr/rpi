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

#include <stdio.h>
#include <stdlib.h>

#include "shell_cmd.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define MAX_CHARS_PER_LINE  4096

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

shell_cmd::shell_cmd(void)
{
  init_members();
}

////////////////////////////////////////////////////////////////

shell_cmd::~shell_cmd(void)
{
}

////////////////////////////////////////////////////////////////

long shell_cmd::check_command(const string cmd,
			      bool &found)
{
  long rc;
  string which_cmd;
  int exit_status;

  // Check if command is found and executable
  which_cmd = "which ";
  which_cmd.append(get_cmd_no_args(cmd));
  which_cmd.append(" > /dev/null");
  rc = execute_shell_cmd(which_cmd, exit_status);
  if (rc == SHELL_CMD_SUCCESS) {
    if (exit_status == 0) {
      found = true;
    }
    else {
      found = false;
    }
  }

  return rc;
}

////////////////////////////////////////////////////////////////

long shell_cmd::execute(const string cmd,
			string &output)
{
  long rc;
  string the_cmd;
  FILE *pipe;
  char buf[MAX_CHARS_PER_LINE];

  output.clear();

  // Check if command exists
  bool cmd_found;
  rc = check_command(cmd, cmd_found);
  if (rc != SHELL_CMD_SUCCESS) {
    return rc;
  }
  if (!cmd_found) {
    return SHELL_CMD_NOT_FOUND;
  }

  // Redirect stderr to stdout
  the_cmd = cmd;
  the_cmd.append(" 2>&1");

  // Open pipe, invoke command
  pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    return SHELL_CMD_FORK_FAILED;
  }

  // Read command standard output
  while (!feof(pipe)) {
    if (fgets(buf, MAX_CHARS_PER_LINE, pipe) != NULL) {
      output.append(buf);
    }
  }

  // Close pipe
  if (pclose(pipe) == -1) {
    return SHELL_CMD_WAIT_FAILED;
  }

  return SHELL_CMD_SUCCESS;
}

////////////////////////////////////////////////////////////////

long shell_cmd::execute(const string cmd,
			int &exit_status)
{
  // Check if command exists
  bool cmd_found;
  long rc = check_command(cmd, cmd_found);
  if (rc != SHELL_CMD_SUCCESS) {
    return rc;
  }
  if (!cmd_found) {
    return SHELL_CMD_NOT_FOUND;
  }

  return execute_shell_cmd(cmd, exit_status);
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void shell_cmd::init_members(void)
{
}

////////////////////////////////////////////////////////////////

string shell_cmd::get_cmd_no_args(const string cmd)
{
  // Get command name without arguments
  size_t found = cmd.find(" ");
  if (found == string::npos) {
    return cmd;
  }
  else {
    return cmd.substr(0, found);
  }
}

////////////////////////////////////////////////////////////////

long shell_cmd::execute_shell_cmd(const string cmd,
				  int &exit_status)
{
  // Execute command
  int rc = system(cmd.c_str());
  if (rc == -1) {
    return SHELL_CMD_FORK_FAILED; // Fork failed
  }
  else {
    // Check exit status
    exit_status = WEXITSTATUS(rc);
    return SHELL_CMD_SUCCESS;     // Command executed
  }
}
