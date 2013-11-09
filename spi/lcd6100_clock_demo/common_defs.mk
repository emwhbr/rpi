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

RASPI_DIR = ../../libraspi
RASPI_OBJ_DIR = $(RASPI_DIR)/obj

LCD6100_DIR = ../../liblcd6100
LCD6100_OBJ_DIR = $(LCD6100_DIR)/obj
LCD6100_INC_DIR = $(LCD6100_DIR)/src

APP_NAME = $(OBJ_DIR)/lcd6100_clock_demo_$(KIND).$(ARCH)

# ----- Compiler flags

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEBUG_PRINTS)

LINK_FLAGS = $(CFLAGS)
COMP_FLAGS = $(LINK_FLAGS) -c

# ----- Includes

INCLUDE = -I $(INC_DIR) -I $(LCD6100_INC_DIR)

# ----- Linker paths

LIB_DIRS = -L $(RASPI_OBJ_DIR) -L $(LCD6100_OBJ_DIR)

# ----- Linker libraries

LIBLCD6100 = -llcd6100 -lraspi

LIBSX = -lstdc++ -lgcc -lpthread -lrt -lm
LIBS  = $(LIBLCD6100) $(LIBSX)

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h

$(OBJ_DIR)/%.o : %.c
	$(CC) $(COMP_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR)/%.o : %.cpp
	$(CPP) $(COMP_FLAGS) $(INCLUDE) -o $@ $<
