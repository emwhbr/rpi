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

#ifndef __REDROBD_ALIVE_THREAD_H__
#define __REDROBD_ALIVE_THREAD_H__

#include "cyclic_thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_alive_thread : public cyclic_thread {

 public:
  redrobd_alive_thread(string thread_name,
		       double frequency);
  ~redrobd_alive_thread(void);

 protected:
  virtual long setup(void);   // Implements pure virtual function from base class
  virtual long cleanup(void); // Implements pure virtual function from base class

  virtual long cyclic_execute(void); // Implements pure virtual function from base class
    
 private:
  // Toggle status LED on/off
  bool m_led_alive_activate;

  void init_members(void);
};

#endif // __REDROBD_ALIVE_THREAD_H__
