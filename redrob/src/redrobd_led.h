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

#ifndef __REDROBD_LED_H__
#define __REDROBD_LED_H__

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////
extern void redrobd_led_initialize(void);
extern void redrobd_led_finalize(void);

extern void redrobd_led_sysfail(bool activate);
extern void redrobd_led_alive(bool activate);
extern void redrobd_led_bat_low(bool activate);

#endif // __REDROBD_LED_H__
