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
	$(OBJ_DIR)/lcd6100_font.o \
	$(OBJ_DIR)/lcd6100_font_small.o \
	$(OBJ_DIR)/lcd6100_font_medium.o \
	$(OBJ_DIR)/lcd6100_font_large.o

# ----- Targets

.PHONY : lib_clean

$(LIB_NAME) : $(OBJS)
	$(AR) rcs $(LIB_NAME) $(OBJS)

lib_clean :
	rm -f $(OBJS) $(LIB_NAME) *~
