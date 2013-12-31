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

#ifndef __CYCLIC_THREAD_H__
#define __CYCLIC_THREAD_H__

#include "thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class cyclic_thread : public thread {

 public:
  cyclic_thread(string thread_name,
		double frequency);
  ~cyclic_thread(void);

  double get_frequency(void);

 protected:
  virtual long setup(void) = 0;    // Pure virtual function
  virtual long execute(void *arg); // Implements pure virtual function from base class
  virtual long cleanup(void) = 0;  // Pure virtual function

  virtual long cyclic_execute(void) = 0; // Pure virtual function
    
 private:
  double m_frequency;
};

#endif // __CYCLIC_THREAD_H__
