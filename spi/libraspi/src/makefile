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

OBJS = $(OBJ_DIR)/raspi.o \
	$(OBJ_DIR)/raspi_core.o \
	$(OBJ_DIR)/raspi_exception.o \
	$(OBJ_DIR)/raspi_io.o

# ----- Targets

.PHONY : lib_clean

$(LIB_NAME) : $(OBJS)
	$(AR) rcs $(LIB_NAME) $(OBJS)

lib_clean :
	rm -f $(OBJS) $(LIB_NAME) *~
