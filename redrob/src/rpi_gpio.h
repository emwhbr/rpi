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

#ifndef __RPI_GPIO_H__
#define __RPI_GPIO_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define RPI_GPIO_MAX_PIN  31  // This class only handles GPIO 0..31

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
// GPIO Function Select Registers
typedef enum {RPI_GPIO_FUNC_INP,  // 000
	      RPI_GPIO_FUNC_OUT,  // 001
              RPI_GPIO_FUNC_ALT5, // 010
	      RPI_GPIO_FUNC_ALT4, // 011
	      RPI_GPIO_FUNC_ALT0, // 100
	      RPI_GPIO_FUNC_ALT1, // 101
	      RPI_GPIO_FUNC_ALT2, // 110
	      RPI_GPIO_FUNC_ALT3, // 111
} RPI_GPIO_FUNCTION;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class rpi_gpio {
  
 public:
  rpi_gpio(void);
  ~rpi_gpio(void);

  void initialize(void);

  void finalize(void);

  void set_function(uint8_t pin,
		    RPI_GPIO_FUNCTION func);

  RPI_GPIO_FUNCTION get_function(uint8_t pin);

  void set_pin_high(uint8_t pin);

  void set_pin_low(uint8_t pin);

  uint8_t get_pin(uint8_t pin);

 private:
  // Memory mapped I/O access
  void              *m_gpio_map;
  volatile uint32_t *m_gpio;

  void init_members(void);

  void check_valid_pin(uint8_t pin);
};

#endif // __RPI_GPIO_H__
