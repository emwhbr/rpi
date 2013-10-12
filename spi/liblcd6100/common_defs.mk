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

RASPI_DIR = ../../libraspi
RASPI_OBJ_DIR = $(RASPI_DIR)/obj
RASPI_INC_DIR = $(RASPI_DIR)/src

EASYBMP_DIR = ../EasyBMP
EASYBMP_INC_DIR = $(EASYBMP_DIR)

LIB_NAME = $(OBJ_DIR)/liblcd6100.a
TEST_APP_NAME = $(OBJ_DIR)/test_liblcd6100_$(KIND).$(ARCH)

# ----- Compiler flags

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)
LIBLCD6100FLAGS = $(DEBUG_PRINTS)

LINK_FLAGS = $(CFLAGS) $(LIBLCD6100FLAGS)
COMP_FLAGS = $(LINK_FLAGS) -c

# ----- Includes

LIBLCD6100_INCLUDE = -I $(INC_DIR) -I $(RASPI_INC_DIR) -I $(EASYBMP_INC_DIR)

INCLUDE = $(LIBLCD6100_INCLUDE)

# ----- Linker paths

LD_LIBLCD6100 = -L $(OBJ_DIR) -L $(RASPI_OBJ_DIR)
LIB_DIRS      = $(LD_LIBLCD6100)

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
