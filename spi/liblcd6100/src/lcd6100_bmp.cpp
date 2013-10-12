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

#include "lcd6100_bmp.h"
#include "lcd6100_exception.h"

// Implementation notes:
// 1. This class utilizes "EasyBMP bitmap library" (ver. 1.06)
//    http://easybmp.sourceforge.net
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_bmp::lcd6100_bmp(string file_name)
{
  m_file_name = file_name;
  m_parsed = false;

  m_height = 0;
  m_width  = 0;

  SetEasyBMPwarningsOff(); // No terminal output from EasyBMP
}

/////////////////////////////////////////////////////////////////////////////

lcd6100_bmp::~lcd6100_bmp(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_bmp::parse(bool scale_to_fit)
{
  // Check if already parsed
  if (m_parsed) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
	      "Image already parsed, file '%s'",
	      m_file_name.c_str());    
  }
  
  // File header
  BMFH bmfh = GetBMFH( m_file_name.c_str() );
  if ( bmfh.bfType != 0x4d42 ) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
	      "File '%s' not a BMP file",
	      m_file_name.c_str());    
  }

  // Work with EasyBMP object
  if (! m_bmp.ReadFromFile( m_file_name.c_str() ) ) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
	      "Error creating BMP from file '%s'",
	      m_file_name.c_str()); 
  }

  // Check that BMP properties are OK

  // Check geometry ...
  if (scale_to_fit) {

    // ... Scale if necessary

    if ( m_bmp.TellHeight() > LCD6100_BMP_MAX_HEIGHT ) {
      // Rescale (preserving aspect ratio) to LCD height
      if (! Rescale(m_bmp, 'H', LCD6100_BMP_MAX_HEIGHT) ) {
	THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
		  "BMP file '%s', failed to scale height",
		  m_file_name.c_str());
      }
    }
    if ( m_bmp.TellWidth() > LCD6100_BMP_MAX_WIDTH ) {
      // Rescale (preserving aspect ratio) to LCD width
      if (! Rescale(m_bmp, 'W', LCD6100_BMP_MAX_WIDTH) ) {
	THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
		  "BMP file '%s', failed to scale width",
		  m_file_name.c_str());
      }
    }

    // Limit BMP to fit screen
    m_height = (m_bmp.TellHeight() > LCD6100_BMP_MAX_HEIGHT ? \
		LCD6100_BMP_MAX_HEIGHT : m_bmp.TellHeight());

    m_width  = (m_bmp.TellWidth() > LCD6100_BMP_MAX_WIDTH ? \
		LCD6100_BMP_MAX_WIDTH : m_bmp.TellWidth());
  }
  else {

    // ... No scaling wanted

    // Check geometry
    if ( (m_bmp.TellHeight() > LCD6100_BMP_MAX_HEIGHT) ||
	 (m_bmp.TellWidth() > LCD6100_BMP_MAX_WIDTH) ) {
      THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
		"BMP file '%s', h x w = %d x %d not supported unscaled",
		m_file_name.c_str(), m_bmp.TellHeight(), m_bmp.TellWidth());
    }

    m_height = m_bmp.TellHeight();
    m_width  = m_bmp.TellWidth();
  }

  // Check supported colours : 12-bit or 24-bit colours
  if ((m_bmp.TellBitDepth() != 12) &&
      (m_bmp.TellBitDepth() != 24)) {
     THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
	      "BMP file '%s', %d bit colours not supported",
	       m_file_name.c_str(), m_bmp.TellBitDepth());
  }

  m_parsed = true;
}

/////////////////////////////////////////////////////////////////////////////

LCD6100_COLOUR lcd6100_bmp::get_pixel(unsigned i, unsigned j)
{
  // Check if not parsed
  if (!m_parsed) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
	      "Image not parsed, file=%s",
	      m_file_name.c_str());    
  }

  // Check that pixel exists
  if ( (i > (m_width - 1)) ||
       (j > (m_height - 1)) ) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BMP_IMAGE_ERROR,
	      "BMP file '%s', non-existing pixel at (%u,%u)",
	      m_file_name.c_str(), i, j); 
  }

  // Get pixel data (colour value)
  RGBApixel rgba = m_bmp.GetPixel(i, j);

  LCD6100_COLOUR colour;

  if (m_bmp.TellBitDepth() == 12) {
    // Native LCD6100 12-bit colour
    colour.bs.red   = rgba.Red;
    colour.bs.green = rgba.Green;
    colour.bs.blue  = rgba.Blue;
  }
  else {
    // Assume 24-bit colour, convert to 12-bit
    colour.bs.red   = rgba.Red / 16;
    colour.bs.green = rgba.Green / 16;
    colour.bs.blue  = rgba.Blue / 16;
  }

  return colour;
}
