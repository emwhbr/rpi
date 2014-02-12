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

#ifndef __RPI_HW_H__
#define __RPI_HW_H__

// GPIO header P1 pin names (ModelB) mapped to BCM2835 GPIO signals
#define PIN_P1_03    2   // I2C1_SDA
#define PIN_P1_05    3   // I2C1_SCL
#define PIN_P1_07    4
#define PIN_P1_08   14   // UART0_TXD
#define PIN_P1_10   15   // UART0_RXD 
#define PIN_P1_11   17
#define PIN_P1_12   18
#define PIN_P1_13   27
#define PIN_P1_15   22
#define PIN_P1_16   23
#define PIN_P1_18   24
#define PIN_P1_19   10   // SPI0_MOSI
#define PIN_P1_21    9   // SPI0_MISO
#define PIN_P1_22   25
#define PIN_P1_23   11   // SPI0_CLK
#define PIN_P1_24    8   // SPI0_CE0
#define PIN_P1_26    7   // SPI0_CE1

// Status indicators (LEDs)
#define PIN_SYSFAIL   PIN_P1_12
#define PIN_ALIVE     PIN_P1_03
#define PIN_BAT_LOW   PIN_P1_05

// Remote Control
#define PIN_RF_IN_0   PIN_P1_07
#define PIN_RF_IN_1   PIN_P1_11
#define PIN_RF_IN_2   PIN_P1_13
#define PIN_RF_IN_3   PIN_P1_15

// Motor Control
#define PIN_L293D_1A  PIN_P1_16
#define PIN_L293D_2A  PIN_P1_18
#define PIN_L293D_3A  PIN_P1_22
#define PIN_L293D_4A  PIN_P1_24

// MCP3008 A/D Converter
#define MCP3008_SPI_DEV      "/dev/spidev0.1" // SPI0_CE1
#define MCP3008_SPI_SPEED    64000            // SPI bitrate (Hz)
#define MCP3008_REF_VOLTAGE  3.3              // Reference voltage

#define MCP3008_CHN_SHUTDOWN   0        // Channel - Shutdown
#define MCP3008_CHN_CONT_STEER 1        // Channel - Continuous steering
#define MCP3008_CHN_VBAT       7        // Channel - Battery voltage
#define MCP3008_CHN_VBAT_SF    0.296837 //           Scale factor: R2 / (R1 + R2)
                                        //           R1=86.7k, R2=36.6k
#endif // __RPI_HW_H__
