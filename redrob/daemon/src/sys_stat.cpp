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
#include <string.h>
#include <fstream>

#include "sys_stat.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define MAX_CHARS_PER_LINE   4096
#define MAX_TOKENS_PER_LINE  800
#define DELIMITER            " "

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

sys_stat::sys_stat(void)
{
  init_members();
}

////////////////////////////////////////////////////////////////

sys_stat::~sys_stat(void)
{
}

////////////////////////////////////////////////////////////////

long sys_stat::reset_interval_cpu_load(void)
{
  // Save current values, start values for interval
  if ( get_current_cpu_load(m_last_cpu_user,
			    m_last_cpu_nice,
			    m_last_cpu_system,
			    m_last_cpu_idle,
			    m_last_cpu_iowait,
			    m_last_cpu_irq,
			    m_last_cpu_softirq) != SYS_STAT_SUCCESS ) {
    return SYS_STAT_FAILURE;
  }

  return SYS_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sys_stat::get_interval_cpu_load(float &value)
{
  // Current values
  unsigned long long int cpu_user;
  unsigned long long int cpu_nice;
  unsigned long long int cpu_system;
  unsigned long long int cpu_idle;
  unsigned long long int cpu_iowait;
  unsigned long long int cpu_irq;
  unsigned long long int cpu_softirq;

  if ( get_current_cpu_load(cpu_user,
			    cpu_nice,
			    cpu_system,
			    cpu_idle,
			    cpu_iowait,
			    cpu_irq,
			    cpu_softirq) != SYS_STAT_SUCCESS ) {
    return SYS_STAT_FAILURE;
  }

  // Calculate load during this interval
  unsigned long long total_delta;
  unsigned long long usage_delta;

  total_delta =
    (cpu_user - m_last_cpu_user) +
    (cpu_nice - m_last_cpu_nice) +
    (cpu_system - m_last_cpu_system) +
    (cpu_idle - m_last_cpu_idle) +
    (cpu_iowait - m_last_cpu_iowait) +
    (cpu_irq - m_last_cpu_irq) +
    (cpu_softirq - m_last_cpu_softirq);

  usage_delta = total_delta - (cpu_idle - m_last_cpu_idle);

  // Check for to small interval
  if (usage_delta == 0 || total_delta == 0) {
    return SYS_STAT_FAILURE;
  }

  value = (float)( (double)(usage_delta) / (double)(total_delta) ) * 100.0;

  return SYS_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sys_stat::reset_interval_irq(void)
{
  // Save current value, start value for interval
  if ( get_current_irq(m_last_irq) != SYS_STAT_SUCCESS ) {
    return SYS_STAT_FAILURE;
  }

  return SYS_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sys_stat::get_interval_irq(unsigned &value)
{
  // Current value
  unsigned long long int irq;

  if ( get_current_irq(irq) != SYS_STAT_SUCCESS ) {
    return SYS_STAT_FAILURE;
  }

  // Calculate irq's during this interval
  value = (unsigned)(irq - m_last_irq);

  return SYS_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sys_stat::get_mem_used_kb(unsigned &value)
{
  bool found;
  unsigned nr_tokens;

  char buf[MAX_CHARS_PER_LINE];

  // Array to store memory addresses of the tokens
  const char* token_list[MAX_TOKENS_PER_LINE] = {};
  
  unsigned long long mem_total;
  unsigned long long mem_free;

  // Get total mem
  find_line("/proc/meminfo",
	    "MemTotal:",
	    found,
	    buf,
	    token_list,
	    nr_tokens);

  if (!found || nr_tokens != 3) {
    return SYS_STAT_FAILURE;
  }
 
  // See man 5 proc for details
  mem_total = atoll(token_list[1]);
  
  // Get free mem
  find_line("/proc/meminfo",
	    "MemFree:",
	    found,
	    buf,
	    token_list,
	    nr_tokens);

  if (!found || nr_tokens != 3) {
    return SYS_STAT_FAILURE;
  }
 
  // See man 5 proc for details
  mem_free = atoll(token_list[1]);

  // Calculate used mem (kb)
  value = (unsigned)(mem_total - mem_free);

  return SYS_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sys_stat::get_uptime_sec(unsigned &value)
{
  ifstream fin;

  fin.open("/proc/uptime");
  if (!fin.good()) {
    return SYS_STAT_FAILURE;
  }

  char buf[MAX_CHARS_PER_LINE];
  
  // Read an entire line into buffer    
  fin.getline(buf, MAX_CHARS_PER_LINE);
  fin.close();

  // Parse line, see man 5 proc for details
  float f1;
  float f2;
  if (sscanf(buf, "%f %f", &f1, &f2) != 2) {
    return SYS_STAT_FAILURE;
  }

  value = (unsigned)f1;

  return SYS_STAT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void sys_stat::init_members(void)
{
  m_last_cpu_user    = 0;
  m_last_cpu_nice    = 0;
  m_last_cpu_system  = 0;
  m_last_cpu_idle    = 0;
  m_last_cpu_iowait  = 0;
  m_last_cpu_irq     = 0;
  m_last_cpu_softirq = 0;

  m_last_irq = 0;
}

////////////////////////////////////////////////////////////////

void sys_stat::find_line(string file_name,
			 string first_token,
			 bool &found,
			 char *line_buffer,
			 const char **token_list,
			 unsigned &nr_tokens)
{
  ifstream fin;
  char *save_ptr;

  found = false; // Assume line not found

  fin.open(file_name.c_str());
  if (!fin.good()) {
    return;
  }
  
  // Read each line of the file
  while (!fin.eof() && !found) {
    // Read an entire line into buffer    
    fin.getline(line_buffer, MAX_CHARS_PER_LINE);
    
    // Parse the line into blank-delimited tokens
    nr_tokens = 0; // a for-loop index    
    
    // Parse the line
    token_list[0] = strtok_r(line_buffer, DELIMITER, &save_ptr); // First token

    if (token_list[0]) { // Zero if line is blank
      for (nr_tokens = 1; nr_tokens < MAX_TOKENS_PER_LINE; nr_tokens++) {
	// Subsequent tokens
        token_list[nr_tokens] = strtok_r(0, DELIMITER, &save_ptr);
        if (!token_list[nr_tokens]) {
	  break; // No more tokens
	}
      }
      if (first_token.compare(token_list[0]) == 0) {
	found = true;
      }
    }
  }

  fin.close();
}

////////////////////////////////////////////////////////////////

long sys_stat::get_current_cpu_load(unsigned long long int &cpu_user,
				    unsigned long long int &cpu_nice,
				    unsigned long long int &cpu_system,
				    unsigned long long int &cpu_idle,
				    unsigned long long int &cpu_iowait,
				    unsigned long long int &cpu_irq,
				    unsigned long long int &cpu_softirq)
{
  bool found;
  unsigned nr_tokens;

  char buf[MAX_CHARS_PER_LINE];

  // Array to store memory addresses of the tokens
  const char* token_list[MAX_TOKENS_PER_LINE] = {};

  find_line("/proc/stat",
	    "cpu",
	    found,
	    buf,
	    token_list,
	    nr_tokens);

  if (!found || nr_tokens != 11) {
    return SYS_STAT_FAILURE;
  }

  // See man 5 proc for details
  cpu_user    = atoll(token_list[1]);
  cpu_nice    = atoll(token_list[2]);
  cpu_system  = atoll(token_list[3]);
  cpu_idle    = atoll(token_list[4]);
  cpu_iowait  = atoll(token_list[5]);
  cpu_irq     = atoll(token_list[6]);
  cpu_softirq = atoll(token_list[7]);

  return SYS_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long sys_stat::get_current_irq(unsigned long long int &irq)
{
  bool found;
  unsigned nr_tokens;

  char buf[MAX_CHARS_PER_LINE];

  // Array to store memory addresses of the tokens
  const char* token_list[MAX_TOKENS_PER_LINE] = {};

  find_line("/proc/stat",
	    "intr",
	    found,
	    buf,
	    token_list,
	    nr_tokens);

  if (!found || nr_tokens < 2) {
    return SYS_STAT_FAILURE;
  }

  // See man 5 proc for details
  irq = atoll(token_list[1]);

  return SYS_STAT_SUCCESS;
}
