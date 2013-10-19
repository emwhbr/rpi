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

#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define GPIO_SUCCESS                 0
#define GPIO_FILE_OPERATION_FAILED  -1
#define GPIO_MEMORY_MAP_FAILED      -2
#define GPIO_PIN_NOT_ALLOWED        -3

// GPIO header P1 pin names (ModelB) mapped to BCM2835 GPIO signals
#define GPIO_P1_03    2   // I2C1_SDA
#define GPIO_P1_05    3   // I2C1_SCL
#define GPIO_P1_07    4
#define GPIO_P1_08   14   // UART0_TXD
#define GPIO_P1_10   15   // UART0_RXD 
#define GPIO_P1_11   17
#define GPIO_P1_12   18
#define GPIO_P1_13   27
#define GPIO_P1_15   22
#define GPIO_P1_16   23
#define GPIO_P1_18   24
#define GPIO_P1_19   10   // SPI0_MOSI
#define GPIO_P1_21    9   // SPI0_MISO
#define GPIO_P1_22   25
#define GPIO_P1_23   11   // SPI0_CLK
#define GPIO_P1_24    8   // SPI0_CE0
#define GPIO_P1_26    7   // SPI0_CE1

#define GPIO_MAX_PIN  31  // This class only handles GPIO 0..31

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

// GPIO Function Select Registers
typedef enum {GPIO_FUNC_INP,  // 000
	      GPIO_FUNC_OUT,  // 001
              GPIO_FUNC_ALT5, // 010
	      GPIO_FUNC_ALT4, // 011
	      GPIO_FUNC_ALT0, // 100
	      GPIO_FUNC_ALT1, // 101
	      GPIO_FUNC_ALT2, // 110
	      GPIO_FUNC_ALT3, // 111
} GPIO_FUNCTION;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class gpio {
  
 public:
  gpio(void);
  ~gpio(void);

  long initialize(void);

  long finalize(void);

  long set_function(uint8_t pin,
		    GPIO_FUNCTION func);

  long get_function(uint8_t pin,
		    GPIO_FUNCTION &func);

  long write(uint8_t pin,
	     uint8_t value);

  long read(uint8_t pin,
	    uint8_t &value);

 private:
  // Memory mapped I/O access
  void              *m_gpio_map;
  volatile uint32_t *m_gpio;

  void init_members(void);
};

#endif // __GPIO_H__
