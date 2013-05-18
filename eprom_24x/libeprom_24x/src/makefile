# /*************************************************************
# *                                                            *
# * Copyright (C) Bonden i Nol                                 *
# *                                                            *
# **************************************************************/

include ../common_defs.mk

OBJS = $(OBJ_DIR)/eprom24x.o \
	$(OBJ_DIR)/eprom24x_core.o \
	$(OBJ_DIR)/eprom24x_exception.o \
	$(OBJ_DIR)/eprom24x_io.o

# ----- Targets

.PHONY : lib_clean

$(LIB_NAME) : $(OBJS)
	$(AR) rcs $(LIB_NAME) $(OBJS)

lib_clean :
	rm -f $(OBJS) $(LIB_NAME) *~
