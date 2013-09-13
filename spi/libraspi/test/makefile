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

TEST_OBJS = $(OBJ_DIR)/test_libraspi.o

# ----- Targets

.PHONY : app_clean

$(TEST_APP_NAME) : $(TEST_OBJS) $(LIB_NAME)
	$(CC) $(LINK_FLAGS) -o $(TEST_APP_NAME) $(TEST_OBJS) $(LIB_DIRS) $(LIBS)

app_clean :
	rm -f $(TEST_OBJS) $(SIM_OBJS) $(OBJ_DIR)/*.$(ARCH) *~
