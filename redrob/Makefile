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

# ----- Toolchain setup

ARCH ?= arm

TC_PREFIX ?= arm-unknown-linux-gnueabihf-

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
CFG_DIR = ./cfg
SCRIPT_DIR = ./script

DAEMON_OBJS = $(OBJ_DIR)/redrobd_main.o \
              $(OBJ_DIR)/redrobd.o \
              $(OBJ_DIR)/redrobd_core.o \
              $(OBJ_DIR)/redrobd_log.o \
              $(OBJ_DIR)/redrobd_ctrl_thread.o \
              $(OBJ_DIR)/redrobd_alive_thread.o \
              $(OBJ_DIR)/redrobd_voltage_monitor_thread.o \
              $(OBJ_DIR)/redrobd_cfg_file.o \
              $(OBJ_DIR)/redrobd_error_utility.o \
              $(OBJ_DIR)/redrobd_thread_utility.o \
              $(OBJ_DIR)/redrobd_gpio.o \
              $(OBJ_DIR)/redrobd_led.o \
              $(OBJ_DIR)/redrobd_remote_ctrl.o \
              $(OBJ_DIR)/redrobd_motor_ctrl.o \
              $(OBJ_DIR)/redrobd_mc_cont_steer.o \
              $(OBJ_DIR)/redrobd_mc_non_cont_steer.o \
              $(OBJ_DIR)/rpi_gpio.o \
              $(OBJ_DIR)/redrobd_hw_cfg.o \
              $(OBJ_DIR)/mcp3008_io.o \
              $(OBJ_DIR)/daemon_utility.o \
              $(OBJ_DIR)/cfg_file.o \
              $(OBJ_DIR)/excep.o \
              $(OBJ_DIR)/delay.o \
              $(OBJ_DIR)/timer.o \
              $(OBJ_DIR)/thread.o \
              $(OBJ_DIR)/cyclic_thread.o

DAEMON_NAME = $(OBJ_DIR)/redrobd_$(KIND).$(ARCH)

# ----- Compiler flags

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEBUG_PRINTS)

LINK_FLAGS = $(CFLAGS)
COMP_FLAGS = $(LINK_FLAGS) -c

# ----- Includes

DAEMON_INCLUDE  = -I$(SRC_DIR)

INCLUDE = $(DAEMON_INCLUDE)

# ----- Linker libraries

LIBSX = -lpthread -lrt -lstdc++
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

daemon : $(DAEMON_OBJS)
	$(CC) $(LINK_FLAGS) -o $(DAEMON_NAME) $(DAEMON_OBJS) $(LIBS)

all : daemon

clean :
	rm -f $(DAEMON_OBJS)
	rm -f $(OBJ_DIR)/*.$(ARCH)
	rm -f $(SRC_DIR)/*~
	rm -f $(CFG_DIR)/*~
	rm -rf $(SCRIPT_DIR)/*~
	rm -rf *~

help:
	@echo "Usage: make clean"
	@echo "       make daemon"
	@echo "       make all"
