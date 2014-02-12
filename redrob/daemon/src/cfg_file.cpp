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

#include "cfg_file.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define CFG_FILE_MAX_LINE_LENGTH  100

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

cfg_file::cfg_file(string file_name)
{
  m_file_name = file_name;
}

////////////////////////////////////////////////////////////////

cfg_file::~cfg_file(void)
{
  // Free any memory allocated by 'set_default_item_value'
  for (unsigned i=0; i < m_item_list.size(); i++) {
    delete m_item_list[i];
  }
}

////////////////////////////////////////////////////////////////

long cfg_file::parse(void)
{
  FILE *fd = NULL;
  char row[CFG_FILE_MAX_LINE_LENGTH];

  // Open config file
  fd = fopen(m_file_name.c_str(), "r");
  if (fd == NULL) {
    if (errno == ENOENT) {
      return CFG_FILE_FILE_NOT_FOUND;
    }    
    return CFG_FILE_FILE_IO_ERROR;
  }

  // Parse file, line by line
  while (fgets(row, CFG_FILE_MAX_LINE_LENGTH, fd) != NULL) {

    // Configuration file format:
    // # This is a comment
    // item=value

    // Remove all white spaces and tabs
    string trimmed_row = trim_all(row);

    // Skip commented or empty rows
    if ( (trimmed_row[0] == '#') || (trimmed_row[0] == '\n') ) {
      continue;
    }

    // Check row format
    if (!row_format_ok(trimmed_row)) {
      fclose(fd);
      return CFG_FILE_BAD_FILE_FORMAT;
    }

    strncpy(row, trimmed_row.c_str(), CFG_FILE_MAX_LINE_LENGTH);

    char *item = strtok(row, "=");
    if (item != NULL) {
      char *value = strtok(NULL, "\r\n");
      if (value != NULL) {
	long rc = populate_item(item, value);
	if (rc != CFG_FILE_SUCCESS) {
	  fclose(fd);
	  return rc;
	}
      }
    }
  }

  // Check if error reading from file
  if (ferror(fd)) {
    fclose(fd);
    return CFG_FILE_FILE_IO_ERROR;
  }

  fclose(fd);

  return CFG_FILE_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

string cfg_file::trim_all(const string &row)
{
  string s = row;

  if(s.size() == 0) {
    return s;
  }
  
  int val = 0;

  // Remove all white spaces and tabs
  for (unsigned cur = 0; cur < s.size(); cur++) {
    if( (s[cur] != ' ') &&
	(s[cur] != '\t')) {
      s[val] = s[cur];
      val++;
    }
  }
  s.resize(val);

  return s;
}

////////////////////////////////////////////////////////////////

bool cfg_file::row_format_ok(const string &row)
{
  // Minimum requirement is 4 chars => 'a=b\n'
  // and row must include '='
  if ( (row.size() < 4) ||
       (row.find('=') == string::npos) ) {
    return false;
  }

  // '=' must not be first or last char
  if ( (row.find('=') == 0) ||
       (row.find('=') == (row.size() - 2)) ) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////

long cfg_file::populate_item(const char *item,
			     const char *value)
{
  // Try to find item in list.
  // If item is found then update value properly.
  for (unsigned i=0; i < m_item_list.size(); i++) {
    if (m_item_list[i]->m_name == item) {

      // Determine how to interpret value
      typed_item<string> *item_string;
      typed_item<double> *item_double;
      typed_item<int> *item_int;
      typed_item<bool> *item_bool;
      item_string = dynamic_cast<typed_item<string>*>(m_item_list[i]);
      item_double = dynamic_cast<typed_item<double>*>(m_item_list[i]);
      item_int    = dynamic_cast<typed_item<int>*>(m_item_list[i]);
      item_bool   = dynamic_cast<typed_item<bool>*>(m_item_list[i]);

      // Interpret as string      
      // No error handling needed, everything can be seen as a strin
      // Format specifier not used
      if (item_string) {
	item_string->m_value = value;
	break;
      }

      // Interpret as double
      if (item_double) {
	if (!is_string_type_t<double>(value, item_double->m_f)) {
	  return CFG_FILE_BAD_VALUE_FORMAT;
	}
	from_string<double>(item_double->m_value,
			    value,
			    item_double->m_f);
	break;
      }

      // Interpret as int
      if (item_int) {
	if (!is_string_type_t<int>(value, item_int->m_f)) {	  
	  return CFG_FILE_BAD_VALUE_FORMAT;
	}
	from_string<int>(item_int->m_value,
			 value,
			 item_int->m_f);
	break;
      }

      // Interpret as bool
      if (item_bool) {
	if (!is_string_type_t<bool>(value, item_bool->m_f)) {
	  return CFG_FILE_BAD_VALUE_FORMAT;
	}
	from_string<bool>(item_bool->m_value,
			  value,
			  item_bool->m_f);
	break;
      }
    }
  }

  return CFG_FILE_SUCCESS;
}
