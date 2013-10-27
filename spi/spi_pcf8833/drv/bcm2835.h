/************************************************************************
 *                                                                      *
 * bcm2835.h                                                            *
 *                                                                      *
 * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
 *                                                                      *
 * Definitions for the Broadcom BCM2835 system on a chip (SoC).         *
 *                                                                      *
 * References:                                                          *
 * [1] Datasheet "BCM2835 ARM Peripherals" (Broadcom, 2012)             *
 *     References to this document is according to: BCM-<section>       *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 ************************************************************************/

#ifndef __BCM2835_H__
#define __BCM2835_H__

/*
 * BCM-1.2 (Memory map), BCM-6.1 (GPIO)
 */
#define BCM2835_PERI_BASE 0x20000000
#define GPIO_BASE_ADDR    (BCM2835_PERI_BASE + 0x200000) /* GPIO registers */

#define GPIO_SIZE_BYTES  0xb4 /* 0x7e20_0000 -- 0x7e20_00b3 */

/*
 * GPIO registers, BCM-6.1 (GPIO)
 * Register offsets are relative to base address of GPIO
 */
#define GPFSEL0  0x00  /* GPIO Function Select 0  */
#define GPFSEL1  0x04  /* GPIO Function Select 1  */
#define GPFSEL2  0x08  /* GPIO Function Select 2  */
#define GPFSEL3  0x0c  /* GPIO Function Select 3  */
#define GPFSEL4  0x10  /* GPIO Function Select 4  */
#define GPFSEL5  0x14  /* GPIO Function Select 5  */
#define GPSET0   0x1c  /* GPIO Pin Output Set 0   */
#define GPSET1   0x20  /* GPIO Pin Output Set 1   */
#define GPCLR0   0x28  /* GPIO Pin Output Clear 0 */
#define GPCLR1   0x2c  /* GPIO Pin Output Clear 1 */

#endif /* __BCM2835_H__ */
