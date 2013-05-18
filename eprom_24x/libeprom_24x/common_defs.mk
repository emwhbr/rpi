# /*************************************************************
# *                                                            *
# * Copyright (C) Bonden i Nol                                 *
# *                                                            *
# **************************************************************/

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

LIB_NAME = $(OBJ_DIR)/libeprom24x.a
TEST_APP_NAME = $(OBJ_DIR)/test_libeprom24x_$(KIND).$(ARCH)

# ----- Compiler flags

CFLAGS = -Wall -Werror
CFLAGS += $(OPTIMIZE)
LIBEPROM24xFLAGS = $(DEBUG_PRINTS)

LINK_FLAGS = $(CFLAGS) $(LIBEPROM24xFLAGS)
COMP_FLAGS = $(LINK_FLAGS) -c

# ----- Includes

LIBEPROM24x_INCLUDE  = -I $(INC_DIR)

INCLUDE = $(LIBEPROM24x_INCLUDE)

# ----- Linker paths

LD_LIBEPROM24x  = -L $(OBJ_DIR)
LIB_DIRS        = $(LD_LIBEPROM24x)

# ----- Linker libraries

LIBEPROM24x = -leprom24x

LIBSX = -lstdc++ -lgcc -lpthread -lrt
LIBS  = $(LIBEPROM24x) $(LIBSX)

# ------ Build rules

.SUFFIXES:
.SUFFIXES: .c .cpp .o .h

$(OBJ_DIR)/%.o : %.c
	$(CC) $(COMP_FLAGS) $(INCLUDE) -o $@ $<

$(OBJ_DIR)/%.o : %.cpp
	$(CPP) $(COMP_FLAGS) $(INCLUDE) -o $@ $<
