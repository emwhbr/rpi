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

include ../common_defs.mk

OBJS = $(OBJ_DIR)/lcd6100.o \
	$(OBJ_DIR)/lcd6100_core.o \
	$(OBJ_DIR)/lcd6100_exception.o \
	$(OBJ_DIR)/lcd6100_delay.o \
	$(OBJ_DIR)/lcd6100_io.o \
	$(OBJ_DIR)/lcd6100_io_bitbang.o \
	$(OBJ_DIR)/lcd6100_io_raspi.o \
        $(OBJ_DIR)/lcd6100_gpio.o \
	$(OBJ_DIR)/lcd6100_font.o \
	$(OBJ_DIR)/lcd6100_font_small.o \
	$(OBJ_DIR)/lcd6100_font_medium.o \
	$(OBJ_DIR)/lcd6100_font_large.o \
	$(OBJ_DIR)/lcd6100_bmp.o

EASYBMP_SRC_DIR = $(EASYBMP_DIR)
EASYBMP_OBJ = $(OBJ_DIR)/EasyBMP.o

# ----- Targets

.PHONY : lib_clean

$(EASYBMP_OBJ) :
	$(CPP) -Wall $(OPTIMIZE) -I $(EASYBMP_INC_DIR) -c $(EASYBMP_SRC_DIR)/EasyBMP.cpp -o $(EASYBMP_OBJ)

$(LIB_NAME) : $(OBJS) $(EASYBMP_OBJ)
	$(AR) rcs $(LIB_NAME) $(OBJS) $(EASYBMP_OBJ)

lib_clean :
	rm -f $(OBJS) $(EASYBMP_OBJ) $(LIB_NAME) *~
