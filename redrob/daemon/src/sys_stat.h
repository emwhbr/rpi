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

#ifndef __SYS_STAT_H__
#define __SYS_STAT_H__

#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define SYS_STAT_SUCCESS   0
#define SYS_STAT_FAILURE  -1

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class sys_stat {

 public:
  sys_stat(void);
  ~sys_stat(void);
  
  long reset_interval_cpu_load(void);
  long get_interval_cpu_load(float &value);

  long reset_interval_irq(void);
  long get_interval_irq(unsigned &value);

  long get_mem_used_kb(unsigned &value);

  long get_uptime_sec(unsigned &value);

 private:
  // Keep track of CPU load
  unsigned long long int m_last_cpu_user;
  unsigned long long int m_last_cpu_nice;
  unsigned long long int m_last_cpu_system;
  unsigned long long int m_last_cpu_idle;
  unsigned long long int m_last_cpu_iowait;
  unsigned long long int m_last_cpu_irq;
  unsigned long long int m_last_cpu_softirq;

  // Keep track of IRQ
  unsigned long long int m_last_irq;

  void init_members(void);

  void find_line(string file_name,
		 string first_token,
		 bool &found,
		 char *line_buffer,
		 const char **token_list,
		 unsigned &nr_tokens);

  long get_current_cpu_load(unsigned long long int &cpu_user,
			    unsigned long long int &cpu_nice,
			    unsigned long long int &cpu_system,
			    unsigned long long int &cpu_idle,
			    unsigned long long int &cpu_iowait,
			    unsigned long long int &cpu_irq,
			    unsigned long long int &cpu_softirq);

  long get_current_irq(unsigned long long int &irq);
};

#endif // __SYS_STAT_H__
