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

#ifndef __REDROBD_LOG_H__
#define __REDROBD_LOG_H__

#include <pthread.h>
#include <stdint.h>
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

#define redrobd_log_initialize redrobd_log::instance()->initialize
#define redrobd_log_finalize   redrobd_log::instance()->finalize
#define redrobd_log_writeln    redrobd_log::instance()->writeln

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_log {

 public:
  ~redrobd_log(void);
  static redrobd_log* instance(void);

  void initialize(string logfile,
		  bool log_stdout);
  void finalize(void);

  void writeln(string str);

 private:
  static redrobd_log *m_instance;
  string             m_logfile;
  bool               m_log_stdout;
  int                m_fd;
  pthread_mutex_t    m_write_mutex;

  redrobd_log(void); // Private constructor
                     // so it can't be called

  void get_date_time_prefix(char *buffer, unsigned len);

  void write_all(int fd,
		 const uint8_t *data,
		 unsigned nbytes);
};

#endif // __REDROBD_LOG_H__
