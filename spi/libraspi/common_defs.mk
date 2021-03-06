# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

# ----- Toolchain setup

ARCH = arm

TC_PREFIX = arm-unknown-linux-gnueabihf-

AR  = $(TC_PREFIX)ar
CC  = $(TC_PREFIX)gcc
CPP = $(TC_PREFIX)g++
AS  = $(TC_PREFIX)gcc
LD  = $(TC_PREFIX)gcc

# ----- Naive setup

ifeq "$(BUILD_TYPE)" "RELEASE"
	OPTIMIZE = -O3
	KIND = rel
else 
	OPTIMIZE = -O0 -g3
	KIND = dbg
	DEBUG_PRINTS = -DDEBUG_PRINTS
endif

OBJ_DIR = ../obj
INC_DIR = ../src
TEST_DIR = ../test

LIB_NAME = $(OBJ_DIR)/libraspi.a
TEST_APP_NAME = $(OBJ_DIR)/test_libraspi_$(KIND).$(ARCH)

# ----- Compiler flags

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)
LIBRASPIFLAGS = $(DEBUG_PRINTS)

LINK_FLAGS = $(CFLAGS) $(LIBRASPIFLAGS)
COMP_FLAGS = $(LINK_FLAGS) -c

# ----- Includes

LIBRASPI_INCLUDE  = -I $(INC_DIR)

INCLUDE = $(LIBRASPI_INCLUDE)

# ----- Linker paths

LD_LIBRASPI  = -L $(OBJ_DIR)
LIB_DIRS     = $(LD_LIBRASPI)

# ----- Linker libraries

LIBRASPI = -lraspi

LIBSX = -lstdc++ -lgcc -lpthread -lrt
LIBS  = $(LIBRASPI) $(LIBSX)

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h

$(OBJ_DIR)/%.o : %.c
	$(CC) $(COMP_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR)/%.o : %.cpp
	$(CPP) $(COMP_FLAGS) $(INCLUDE) -o $@ $<
