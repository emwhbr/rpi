# /***********************************************************************
# *                                                                      *
# * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
# *                                                                      *
# * This program is free software; you can redistribute it and/or modify *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation; either version 2 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# ************************************************************************/

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

OBJ_DIR = ./obj
SRC_DIR = ./src

TEST_OBJS = $(OBJ_DIR)/test_hdc1008.o \
              $(OBJ_DIR)/hdc1008_io.o

TEST_NAME = $(OBJ_DIR)/test_hdc1008_$(KIND).$(ARCH)

# ----- Compiler flags

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEBUG_PRINTS)

LINK_FLAGS = $(CFLAGS)
COMP_FLAGS = $(LINK_FLAGS) -c

# ----- Includes

TEST_INCLUDE  = -I$(SRC_DIR)

INCLUDE = $(TEST_INCLUDE)

# ----- Linker libraries

LIBSX = -lstdc++ -lgcc -lpthread -lrt
LIBS  = $(LIBSX)

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(COMP_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CPP) $(COMP_FLAGS) $(INCLUDE) -o $@ $<

# ------ Targets

.PHONY : clean help

test : $(TEST_OBJS)
	$(CC) $(LINK_FLAGS) -o $(TEST_NAME) $(TEST_OBJS) $(LIBS)

all : test

clean :
	rm -f $(TEST_OBJS) $(OBJ_DIR)/*.$(ARCH) $(SRC_DIR)/*~ *~

help:
	@echo "Usage: make clean"
	@echo "       make test"
	@echo "       make all"
