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

#ifndef __LCD_H__
#define __LCD_H__

#include <string>

#include "lcd6100.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd {
  
 public:
  lcd(const char *rev_info,
      LCD6100_IFACE lcd_iface,
      LCD6100_CE lcd_ce,
      uint32_t lcd_speed);

  ~lcd(void);

  void initialize(void);

  void finalize(void);

  void update_digital_time(uint8_t hour,
			   uint8_t min,
			   uint8_t sec);

  void draw_digital_time(void);

  void update_analog_time(uint8_t sec_hand_end_point_row,
			  uint8_t sec_hand_end_point_col);

  void draw_analog_time(void);

 private:
  string m_rev_info;

  // LCD SPI
  LCD6100_IFACE m_lcd_iface;
  LCD6100_CE    m_lcd_ce;
  uint32_t      m_lcd_speed;
  bool          m_lcd_initialized;

  // Keep track of digital time
  uint8_t m_dig_hour;
  uint8_t m_dig_min;
  uint8_t m_dig_sec;

  char m_str_dig_hour[3];
  char m_str_dig_min[3];
  char m_str_dig_sec[3];

  LCD6100_COLOUR m_dig_bg_colour;
  LCD6100_COLOUR m_dig_fg_colour;

  bool m_dig_hour_updated;
  bool m_dig_min_updated;
  bool m_dig_sec_updated;

  // Keep track of analog time
  uint8_t m_ana_sec_hand_end_point_row;
  uint8_t m_ana_sec_hand_end_point_col;

  LCD6100_COLOUR m_ana_bg_colour;
  LCD6100_COLOUR m_ana_fg_colour;

  bool m_ana_sec_updated;
    
  // Private member functions
  void init_members(void);

  void draw_rev_info(void);

  void draw_digital_time_separators(void);

  void draw_analog_time_markers(void);

  void throw_lcd6100_exception(const string why);
};

#endif // __LCD_H__
