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

#ifndef __LCD6100_GPIO_H__
#define __LCD6100_GPIO_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// GPIO header P1 pin names (ModelB) mapped to BCM2835 GPIO signals
#define LCD6100_GPIO_P1_03    2   // I2C1_SDA
#define LCD6100_GPIO_P1_05    3   // I2C1_SCL
#define LCD6100_GPIO_P1_07    4
#define LCD6100_GPIO_P1_08   14   // UART0_TXD
#define LCD6100_GPIO_P1_10   15   // UART0_RXD 
#define LCD6100_GPIO_P1_11   17
#define LCD6100_GPIO_P1_12   18
#define LCD6100_GPIO_P1_13   27
#define LCD6100_GPIO_P1_15   22
#define LCD6100_GPIO_P1_16   23
#define LCD6100_GPIO_P1_18   24
#define LCD6100_GPIO_P1_19   10   // SPI0_MOSI
#define LCD6100_GPIO_P1_21    9   // SPI0_MISO
#define LCD6100_GPIO_P1_22   25
#define LCD6100_GPIO_P1_23   11   // SPI0_CLK
#define LCD6100_GPIO_P1_24    8   // SPI0_CE0
#define LCD6100_GPIO_P1_26    7   // SPI0_CE1

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

// GPIO Function Select Registers
typedef enum {LCD6100_GPIO_FUNC_INP,  // 000
	      LCD6100_GPIO_FUNC_OUT,  // 001
              LCD6100_GPIO_FUNC_ALT5, // 010
	      LCD6100_GPIO_FUNC_ALT4, // 011
	      LCD6100_GPIO_FUNC_ALT0, // 100
	      LCD6100_GPIO_FUNC_ALT1, // 101
	      LCD6100_GPIO_FUNC_ALT2, // 110
	      LCD6100_GPIO_FUNC_ALT3, // 111
} LCD6100_GPIO_FUNCTION;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_gpio {
  
 public:
  lcd6100_gpio(void);
  ~lcd6100_gpio(void);

  void initialize(void);

  void finalize(void);

  void set_function(uint8_t pin,
		    LCD6100_GPIO_FUNCTION func);

  void get_function(uint8_t pin,
		    LCD6100_GPIO_FUNCTION &func);

  void write(uint8_t pin,
	     uint8_t value);

  void read(uint8_t pin,
	    uint8_t &value);

 private:
  // Memory mapped I/O access
  void              *m_gpio_map;
  volatile uint32_t *m_gpio;

  void init_members(void);
};

#endif // __LCD6100_GPIO_H__
