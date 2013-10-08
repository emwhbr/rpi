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

#ifndef __LCD6100_FONT_LARGE_H__
#define __LCD6100_FONT_LARGE_H__

#include "lcd6100_font.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

#define FONT_LARGE_NR_CHARS        96
#define FONT_LARGE_BYTES_PER_CHAR  16

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_font_large : public lcd6100_font {

public:
  lcd6100_font_large(void);
  ~lcd6100_font_large(void);
  
  const uint8_t* get_font_table(char c);

private:
  static const uint8_t m_font_table[FONT_LARGE_NR_CHARS][FONT_LARGE_BYTES_PER_CHAR];
};

#endif // __LCD6100_FONT_LARGE_H__
