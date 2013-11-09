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

OBJS = $(OBJ_DIR)/lcd6100_clock_demo.o \
       $(OBJ_DIR)/lcd.o \
       $(OBJ_DIR)/excep.o \
       $(OBJ_DIR)/delay.o

# ----- Targets

.PHONY : app_clean

$(APP_NAME) : $(OBJS)
	$(CPP) $(LINK_FLAGS) -o $(APP_NAME) $(OBJS) $(LIB_DIRS) $(LIBS)

app_clean :
	rm -f $(OBJS) $(OBJ_DIR)/*.$(ARCH) *~
