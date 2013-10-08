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

#ifndef __LCD6100_HW_H__
#define __LCD6100_HW_H__

// Philips PCF8833 LCD controller commands.
// Data sheet, 2003 Feb 14 (page 8-11)
#define CMD_NOP       0x00
#define CMD_SLEEPIN   0x10
#define CMD_SLEEPOUT  0x11
#define CMD_INVOFF    0x20
#define CMD_INVON     0x21
#define CMD_SETCON    0x25
#define CMD_DISPOFF   0x28
#define CMD_DISPON    0x29
#define CMD_CASET     0x2A
#define CMD_PASET     0x2B
#define CMD_RAMWR     0x2C
#define CMD_COLMOD    0x3A
#define CMD_MADCTL    0x36

#endif // __LCD6100_HW_H__
