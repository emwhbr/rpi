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

#include <stdio.h>
#include <sstream>

#include "lcd.h"
#include "excep.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

// Digital clock
#define DIGITAL_START_ROW  0
#define DIGITAL_START_COL  0
#define DIGITAL_END_ROW   25
#define DIGITAL_END_COL   LCD6100_COL_MAX_ADDR

#define DIGITAL_TIME_ROW    4
#define DIGITAL_HOUR_COL   10
#define DIGITAL_SEP1_COL   42
#define DIGITAL_MIN_COL    58
#define DIGITAL_SEP2_COL   90
#define DIGITAL_SEC_COL   106

#define DIGITAL_BG_COLOUR LCD6100_RGB_NAVY
#define DIGITAL_FG_COLOUR LCD6100_RGB_YELLOW

#define DIGITAL_FONT LCD6100_FONT_LARGE

// Analog clock
#define ANALOG_ORIGO_ROW  77
#define ANALOG_ORIGO_COL  65
#define ANALOG_RADIUS     36

#define ANALOG_00S_ROW  117
#define ANALOG_00S_COL   60
#define ANALOG_15S_ROW   73
#define ANALOG_15S_COL  105
#define ANALOG_30S_ROW   30
#define ANALOG_30S_COL   60
#define ANALOG_45S_ROW   73
#define ANALOG_45S_COL   13

#define ANALOG_BG_COLOUR LCD6100_RGB_BLACK
#define ANALOG_FG_COLOUR LCD6100_RGB_RED

#define ANALOG_FONT LCD6100_FONT_SMALL

// Revision info
#define REV_ROW  120
#define REV_COL   92

#define REV_BG_COLOUR LCD6100_RGB_BLACK
#define REV_FG_COLOUR LCD6100_RGB_GREEN

#define REV_FONT LCD6100_FONT_MEDIUM

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd::lcd(const char *rev_info,
	 LCD6100_IFACE lcd_iface,
	 LCD6100_CE lcd_ce,
	 uint32_t lcd_speed)
{
  m_rev_info = rev_info;

  m_lcd_iface = lcd_iface;
  m_lcd_ce    = lcd_ce;
  m_lcd_speed = lcd_speed;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

lcd::~lcd(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd::initialize(void)
{
  // Init LCD
  if (lcd6100_initialize(m_lcd_iface,
                         m_lcd_ce,
                         m_lcd_speed) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Initialize LCD");
  }
  m_lcd_initialized = true;

  if (lcd6100_clear_screen() != LCD6100_SUCCESS) {
    throw_lcd6100_exception("Clear LCD");
  }

  draw_rev_info();

  // Create digital time area
  LCD6100_COLOUR digital_bg_colour;
  digital_bg_colour.wd = DIGITAL_BG_COLOUR;
  if (lcd6100_draw_rectangle(DIGITAL_START_ROW,
			     DIGITAL_START_COL,
			     DIGITAL_END_ROW,
			     DIGITAL_END_COL,
			     true,
			     digital_bg_colour) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Create digital time area");
  }

  draw_digital_time_separators();
  draw_digital_time();

  // Create analog time area
  LCD6100_COLOUR analog_colour;
  analog_colour.wd = ANALOG_FG_COLOUR;
  if (lcd6100_draw_circle(ANALOG_ORIGO_ROW,
			  ANALOG_ORIGO_COL,
			  ANALOG_RADIUS,
			  analog_colour) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Create analog time area");
  }

  draw_analog_time_markers();
  draw_analog_time();
}

/////////////////////////////////////////////////////////////////////////////

void lcd::finalize(void)
{
  if (!m_lcd_initialized) {
    return;
  }

  if (lcd6100_clear_screen() != LCD6100_SUCCESS) {
    throw_lcd6100_exception("Clear LCD");
  }

  if (lcd6100_finalize() != LCD6100_SUCCESS) {
    throw_lcd6100_exception("Finalize LCD");
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

void lcd::update_digital_time(uint8_t hour,
			      uint8_t min,
			      uint8_t sec)
{
  // Only update time parts if necessary

  if (hour != m_dig_hour) {
    m_dig_hour = hour;
    snprintf(m_str_dig_hour, 3, "%02d", m_dig_hour);
    m_dig_hour_updated = true; // Mark as updated
  }

  if (min != m_dig_min) {
    m_dig_min = min;
    snprintf(m_str_dig_min, 3, "%02d", m_dig_min);
    m_dig_min_updated = true; // Mark as updated
  }

  if (sec != m_dig_sec) {
    m_dig_sec = sec;
    snprintf(m_str_dig_sec, 3, "%02d", m_dig_sec);
    m_dig_sec_updated = true; // Mark as updated
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd::draw_digital_time(void)
{
  // Only draw time parts that has been updated

  if (m_dig_hour_updated) {
    if (lcd6100_write_string(m_str_dig_hour,
			     DIGITAL_TIME_ROW,
			     DIGITAL_HOUR_COL,
			     m_dig_fg_colour,
			     m_dig_bg_colour,
			     DIGITAL_FONT) != LCD6100_SUCCESS) {

      throw_lcd6100_exception("Draw digital hour");
    }
    m_dig_hour_updated = false; // Mark as drawn
  }

  if (m_dig_min_updated) {
    if (lcd6100_write_string(m_str_dig_min,
			     DIGITAL_TIME_ROW,
			     DIGITAL_MIN_COL,
			     m_dig_fg_colour,
			     m_dig_bg_colour,
			     DIGITAL_FONT) != LCD6100_SUCCESS) {

      throw_lcd6100_exception("Draw digital minute");
    }
    m_dig_min_updated = false; // Mark as draw
  }

  if (m_dig_sec_updated) {
    if (lcd6100_write_string(m_str_dig_sec,
			     DIGITAL_TIME_ROW,
			     DIGITAL_SEC_COL,
			     m_dig_fg_colour,
			     m_dig_bg_colour,
			     DIGITAL_FONT) != LCD6100_SUCCESS) {

      throw_lcd6100_exception("Draw digital second");
    }
    m_dig_sec_updated = false; // Mark as drawn
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd::reset_digital_time(void)
{
  update_digital_time(0, 0, 0);
  draw_digital_time();
}

/////////////////////////////////////////////////////////////////////////////

uint8_t lcd::get_analog_origo_row(void)
{
  return ANALOG_ORIGO_ROW;
}

/////////////////////////////////////////////////////////////////////////////

uint8_t lcd::get_analog_origo_col(void)
{
  return ANALOG_ORIGO_COL;
}

/////////////////////////////////////////////////////////////////////////////

uint8_t lcd::get_analog_radius(void)
{
  return ANALOG_RADIUS - 3; // We don't want to overwrite clock circle
}

/////////////////////////////////////////////////////////////////////////////

void lcd::update_analog_time(uint8_t sec_end_row,
			     uint8_t sec_end_col)
{
  // Only update time parts if necessary
  
  if ( (sec_end_row != m_ana_sec_end_row) ||
       (sec_end_col != m_ana_sec_end_col) ) {

    m_ana_old_sec_end_row = m_ana_sec_end_row;
    m_ana_old_sec_end_col = m_ana_sec_end_col;

    m_ana_sec_end_row = sec_end_row;
    m_ana_sec_end_col = sec_end_col;

    m_ana_sec_updated = true; // Mark as updated
  } 
}

/////////////////////////////////////////////////////////////////////////////

void lcd::draw_analog_time(void)
{
  // Only draw time parts that has been updated

  if (m_ana_sec_updated) {

    // Erase old second
    if (lcd6100_draw_line(ANALOG_ORIGO_ROW,
			  ANALOG_ORIGO_COL,
			  m_ana_old_sec_end_row,
			  m_ana_old_sec_end_col,
			  m_ana_bg_colour) != LCD6100_SUCCESS) {

      throw_lcd6100_exception("Erase old analog second");
    }

    // Draw new second
    if (lcd6100_draw_line(ANALOG_ORIGO_ROW,
			  ANALOG_ORIGO_COL,
			  m_ana_sec_end_row,
			  m_ana_sec_end_col,
			  m_ana_fg_colour) != LCD6100_SUCCESS) {

      throw_lcd6100_exception("Draw new analog second");
    }

    m_ana_sec_updated = false; // Mark as drawn
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd::reset_analog_time(void)
{
  // Erase old second
  if (lcd6100_draw_line(ANALOG_ORIGO_ROW,
			ANALOG_ORIGO_COL,
			m_ana_old_sec_end_row,
			m_ana_old_sec_end_col,
			m_ana_bg_colour) != LCD6100_SUCCESS) {
    
    throw_lcd6100_exception("Erase old analog second");
  }

  m_ana_sec_end_row = ANALOG_ORIGO_ROW + get_analog_radius();
  m_ana_sec_end_col = ANALOG_ORIGO_COL;

  m_ana_old_sec_end_row = m_ana_sec_end_row;
  m_ana_old_sec_end_col = m_ana_sec_end_col;

  // Draw reset second
  if (lcd6100_draw_line(ANALOG_ORIGO_ROW,
			ANALOG_ORIGO_COL,
			m_ana_sec_end_row,
			m_ana_sec_end_col,
			m_ana_fg_colour) != LCD6100_SUCCESS) {
    
    throw_lcd6100_exception("Draw reset analog second");
  }

  m_ana_sec_updated = false; // Mark as drawn
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd::init_members(void)
{
  m_lcd_initialized = false;

  m_dig_hour = 0;
  m_dig_min  = 0;
  m_dig_sec  = 0;

  snprintf(m_str_dig_hour, 3, "%02d", m_dig_hour);
  snprintf(m_str_dig_min,  3, "%02d", m_dig_min);
  snprintf(m_str_dig_sec,  3, "%02d", m_dig_sec);

  m_dig_bg_colour.wd = DIGITAL_BG_COLOUR;
  m_dig_fg_colour.wd = DIGITAL_FG_COLOUR;

  m_dig_hour_updated = true;
  m_dig_min_updated  = true;
  m_dig_sec_updated  = true;

  m_ana_sec_end_row = ANALOG_ORIGO_ROW + get_analog_radius();
  m_ana_sec_end_col = ANALOG_ORIGO_COL;

  m_ana_old_sec_end_row = m_ana_sec_end_row;
  m_ana_old_sec_end_col = m_ana_sec_end_col;

  m_ana_bg_colour.wd = ANALOG_BG_COLOUR;
  m_ana_fg_colour.wd = ANALOG_FG_COLOUR;
  
  m_ana_sec_updated = true;
}

/////////////////////////////////////////////////////////////////////////////

void lcd::draw_rev_info(void)
{
  LCD6100_COLOUR rev_bg_colour;
  LCD6100_COLOUR rev_fg_colour;

  rev_bg_colour.wd = REV_BG_COLOUR;
  rev_fg_colour.wd = REV_FG_COLOUR;

  if (lcd6100_write_string(m_rev_info.c_str(),
			   REV_ROW,
			   REV_COL,
			   rev_fg_colour,
			   rev_bg_colour,
			   REV_FONT) != LCD6100_SUCCESS) {
    
    throw_lcd6100_exception("Draw revision");
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd::draw_digital_time_separators(void)
{
  // Time separators: hh : min : sec

  if (lcd6100_write_string(":",
			   DIGITAL_TIME_ROW,
			   DIGITAL_SEP1_COL,
			   m_dig_fg_colour,
			   m_dig_bg_colour,
			   DIGITAL_FONT) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Draw digital time separator 1");
  }

  if (lcd6100_write_string(":",
			   DIGITAL_TIME_ROW,
			   DIGITAL_SEP2_COL,
			   m_dig_fg_colour,
			   m_dig_bg_colour,
			   DIGITAL_FONT) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Draw digital time separator 2");
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd::draw_analog_time_markers(void)
{
  // Time markers: 0(60), 15, 30, 45 seconds
  if (lcd6100_write_string("60",
			   ANALOG_00S_ROW,
			   ANALOG_00S_COL,
			   m_ana_fg_colour,
			   m_ana_bg_colour,
			   ANALOG_FONT) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Draw analog time marker 00s");
  }

  if (lcd6100_write_string("15",
			   ANALOG_15S_ROW,
			   ANALOG_15S_COL,
			   m_ana_fg_colour,
			   m_ana_bg_colour,
			   ANALOG_FONT) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Draw analog time marker 15s");
  }

  if (lcd6100_write_string("30",
			   ANALOG_30S_ROW,
			   ANALOG_30S_COL,
			   m_ana_fg_colour,
			   m_ana_bg_colour,
			   ANALOG_FONT) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Draw analog time marker 30s");
  }
  
  if (lcd6100_write_string("45",
			   ANALOG_45S_ROW,
			   ANALOG_45S_COL,
			   m_ana_fg_colour,
			   m_ana_bg_colour,
			   ANALOG_FONT) != LCD6100_SUCCESS) {

    throw_lcd6100_exception("Draw analog time marker 45s");
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd::throw_lcd6100_exception(const string why)
{
  LCD6100_STATUS status;
  LCD6100_ERROR_STRING error_string;

  ostringstream oss_msg;

  oss_msg << why << " ==>\n";

  if (lcd6100_get_last_error(&status) != LCD6100_SUCCESS) {
    oss_msg << "lcd6100_get_last_error() failed";
    THROW_EXP(0, 0, oss_msg.str().c_str(), NULL);
  }

  if (lcd6100_get_error_string(status.error_code,
			       error_string) != LCD6100_SUCCESS) {
    oss_msg << "lcd6100_get_error_string() failed";
    THROW_EXP(0, 0, oss_msg.str().c_str(), NULL);
  }

  switch (status.error_source) {
  case LCD6100_INTERNAL_ERROR:
    oss_msg << "LCD6100 error source : LCD6100_INTERNAL_ERROR\n";
    break;
  case LCD6100_LINUX_ERROR:
    oss_msg << "LCD6100 error source : LCD6100_LINUX_ERROR\n";
    break;
  default:
    oss_msg << "LCD6100 error source : *** UNKNOWN\n";
  }

  oss_msg << "LCD6100 error code   : " << status.error_code << "\n";
  oss_msg << "LCD6100 error string : " << error_string;

  THROW_EXP(0, 0, oss_msg.str().c_str(), NULL);
}
