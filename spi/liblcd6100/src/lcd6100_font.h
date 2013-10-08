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

#ifndef __LCD6100_FONT_H__
#define __LCD6100_FONT_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_font {

public:
  lcd6100_font(void);
  ~lcd6100_font(void);
  
  uint8_t get_height(void) { return m_height;};	// In pixels
  uint8_t get_width(void)  { return m_width;};  // In pixels
  
  uint8_t get_bytes_per_char(void) { return m_bytes_per_char;};
  
  virtual const uint8_t* get_font_table(char c) = 0;
  
protected:
  uint8_t m_height;
  uint8_t m_width;
  uint8_t m_bytes_per_char;
};

#endif // __LCD6100_FONT_H__
