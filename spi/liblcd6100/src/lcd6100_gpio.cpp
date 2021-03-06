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

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "lcd6100_gpio.h"
#include "lcd6100_exception.h"

// Implementation notes:
// 1. Datasheet "BCM2835 ARM Peripherals" (Broadcom, 2012)
//    Section 1.2 (Memory map) and 6.1 (GPIO).
//
// 2. Raspberry Pi low-level peripherals
//    http://elinux.org/RPi_Low-level_peripherals
// 
// 3. Wiring Pi GPIO library
//    http://wiringpi.com

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

#define BCM2835_PERI_BASE  0x20000000
#define GPIO_BASE          (BCM2835_PERI_BASE + 0x200000) // GPIO registers

#define GPIO_MAX_PIN  31  // This class only handles GPIO 0..31

// GPIO registers, 32-bit offset from GPIO_BASE
#define GPFSEL0_OFFSET32   0
#define GPFSEL1_OFFSET32   1
#define GPFSEL2_OFFSET32   2
#define GPFSEL3_OFFSET32   3
#define GPFSEL4_OFFSET32   4
#define GPFSEL5_OFFSET32   5
#define GPSET0_OFFSET32    7
#define GPSET1_OFFSET32    8
#define GPCLR0_OFFSET32   10
#define GPCLR1_OFFSET32   11
#define GPLEV0_OFFSET32   13
#define GPLEV1_OFFSET32   14

#define PAGE_SIZE  (4*1024)  // Kernel page size

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_gpio::lcd6100_gpio(void)
{
  init_members();
}

/////////////////////////////////////////////////////////////////////////////

lcd6100_gpio::~lcd6100_gpio(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::initialize(void)
{
  const char *mem_dev = "/dev/mem";
  int fd_mem;

  // Open physical memory file abstraction
  fd_mem = open(mem_dev, O_RDWR|O_SYNC);
  if (fd_mem == -1) {
    THROW_LXP(LCD6100_LINUX_ERROR, LCD6100_FILE_OPERATION_FAILED,
	      "open failed, device (%s)", mem_dev);
  }

  // Memory map GPIO registers section of peripherals
  m_gpio_map = mmap(NULL,                  // Any adddress in our space will do
		    PAGE_SIZE,             // Map length
		    PROT_READ | PROT_WRITE,// Enable reading & writting to mapped memory
		    MAP_SHARED,            // Shared with other processes
		    fd_mem,                // File to map
		    GPIO_BASE);            // Offset to GPIO peripheral registers

  if ( m_gpio_map == MAP_FAILED ) {
    close(fd_mem);
    THROW_LXP(LCD6100_LINUX_ERROR, LCD6100_MEMORY_MAP_FAILED,
	      "mmap failed for gpio address(0x%x)", GPIO_BASE);
  }

  // No need to keep descriptor open
  if ( close(fd_mem) == -1 ) {
    munmap(m_gpio_map, PAGE_SIZE);
    THROW_LXP(LCD6100_LINUX_ERROR, LCD6100_FILE_OPERATION_FAILED,
	      "close failed, device (%s)", mem_dev);
  }

  m_gpio = (volatile uint32_t *) m_gpio_map;
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::finalize(void)
{
  if ( munmap(m_gpio_map, PAGE_SIZE) == -1 ) {
    THROW_LXP(LCD6100_LINUX_ERROR, LCD6100_MEMORY_MAP_FAILED,
	      "munmap failed for gpio address(0x%x)", GPIO_BASE);
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::set_function(uint8_t pin,
				LCD6100_GPIO_FUNCTION func)
{
  if (pin > GPIO_MAX_PIN) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "gpio pin(%u) not allowed, max(%u)", pin, GPIO_MAX_PIN);
  }

  long offset32;
  switch (pin / 10) {
  case 0:
    offset32 = GPFSEL0_OFFSET32;  // pin 0..9
    break;
  case 1:
    offset32 = GPFSEL1_OFFSET32;  // pin 10..19
    break;
  case 2:
    offset32 = GPFSEL2_OFFSET32;  // pin 20..29
    break;
  case 3:
    offset32 = GPFSEL3_OFFSET32;  // pin 30..39
    break;
  default:
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "gpio pin(%u) not allowed", pin);
  }

  // Function is 3 bits for each of the 10 pins
  
  uint32_t gpfsel_reg = *(m_gpio + offset32);

  // Always set to INPUT first
  if (func != LCD6100_GPIO_FUNC_INP) {
    gpfsel_reg &= ~( 0x7 << ((pin%10) * 3) ); // Clear function bits for pin
    *(m_gpio + offset32) = gpfsel_reg;
  }
  
  gpfsel_reg = *(m_gpio + offset32);

  gpfsel_reg &= ~( 0x7 << ((pin%10) * 3) ); // Clear function bits for pin
  gpfsel_reg |= ( func << ((pin%10) * 3) ); // Set function bits for pin

  *(m_gpio + offset32) = gpfsel_reg;
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::get_function(uint8_t pin,
				LCD6100_GPIO_FUNCTION &func)
{
  if (pin > GPIO_MAX_PIN) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "gpio pin(%u) not allowed, max(%u)", pin, GPIO_MAX_PIN);
  }

  long offset32;
  switch (pin / 10) {
  case 0:
    offset32 = GPFSEL0_OFFSET32;  // pin 0..9
    break;
  case 1:
    offset32 = GPFSEL1_OFFSET32;  // pin 10..19
    break;
  case 2:
    offset32 = GPFSEL2_OFFSET32;  // pin 20..29
    break;
  case 3:
    offset32 = GPFSEL3_OFFSET32;  // pin 30..39
    break;
  default:
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "gpio pin(%u) not allowed", pin);
  }

  // Function is 3 bits for each of the 10 pins
  func = (LCD6100_GPIO_FUNCTION) ( (*(m_gpio + offset32) >> ((pin%10) * 3)) & 0x7 );
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::write(uint8_t pin,
			 uint8_t value)
{
  if (pin > GPIO_MAX_PIN) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "gpio pin(%u) not allowed, max(%u)", pin, GPIO_MAX_PIN);
  }

  // Note!
  // Writing 0 to a bit has no effect on corresponding pin.
  // This is valid for both set and clear.
  if (value) {
    *(m_gpio + GPSET0_OFFSET32) = 1 << pin;
  }
  else {
    *(m_gpio + GPCLR0_OFFSET32) = 1 << pin;
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::read(uint8_t pin,
			uint8_t &value)
{
  if (pin > GPIO_MAX_PIN) {
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_BAD_ARGUMENT,
	      "gpio pin(%u) not allowed, max(%u)", pin, GPIO_MAX_PIN);
  }

  // Read actual value of pin
  if ( *(m_gpio + GPLEV0_OFFSET32) & (1 << pin)  ) {
    value = 1;
  }
  else {
    value = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd6100_gpio::init_members(void)
{
  m_gpio_map = NULL;
  m_gpio = NULL;
}
