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

#ifndef __LCD6100_BMP_H__
#define __LCD6100_BMP_H__

#include <string>

#include "lcd6100.h"
#include "EasyBMP.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

#define LCD6100_BMP_MAX_HEIGHT (LCD6100_ROW_MAX_ADDR + 1)
#define LCD6100_BMP_MAX_WIDTH  (LCD6100_COL_MAX_ADDR + 1)

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_bmp {

public:
  lcd6100_bmp(string file_name);
  ~lcd6100_bmp(void);

  void parse(bool scale_to_fit);

  unsigned get_height(void) { return m_height;}; // In pixels
  unsigned get_width(void)  { return m_width;};  // In pixels

  // The coordinate system of a BMP file has its origin
  // in the top left corner of the image:
  // (i, j)th pixel is 
  //    i pixels from the left
  //    j pixels from the top
  LCD6100_COLOUR get_pixel(unsigned i, unsigned j);  

protected:
  string m_file_name;
  bool m_parsed;

  unsigned m_height;
  unsigned m_width;

  BMP m_bmp;
};

#endif // __LCD6100_BMP_H__
