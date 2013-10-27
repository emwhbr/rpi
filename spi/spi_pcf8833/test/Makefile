# /***********************************************************************
# *                                                                      *
# * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
# *                                                                      *
# * This program is free software; you can redistribute it and/or modify *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation; either version 2 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# ************************************************************************/

include ../common_defs.mk

# ----- User space test application

TEST_OBJS = test_spi_pcf8833.o

TEST_APP_NAME = test_spi_pcf8833

# ----- Toolchain setup

TC_PREFIX = $(CROSS_COMPILE)

AR  = $(TC_PREFIX)ar
CC  = $(TC_PREFIX)gcc
CPP = $(TC_PREFIX)g++
AS  = $(TC_PREFIX)gcc
LD  = $(TC_PREFIX)gcc

# ----- Compiler flags

OPTIMIZE = -O3
KIND = rel

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)

COMP_FLAGS = $(CFLAGS) -c

# ----- Includes

INCLUDE = -I./

# ----- Linker

LIBS = -lgcc -lpthread -lrt

# ----- Build rules

.SUFFIXES:
.SUFFIXES: .c .o .h

%.o : %.c
	$(CC) $(COMP_FLAGS) $(INCLUDE) -o $@ $<

# ----- Available make targets

.PHONY : clean

test : $(TEST_OBJS)
	$(CC) -o $(TEST_APP_NAME)_$(KIND).$(ARCH) $(TEST_OBJS) $(LIBS)

clean :
	rm -f $(TEST_APP_NAME)_$(KIND).$(ARCH)
	rm -f $(TEST_OBJS)
	rm -f *~
