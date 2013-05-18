# /*************************************************************
# *                                                            *
# * Copyright (C) Bonden i Nol                                 *
# *                                                            *
# **************************************************************/

include ../common_defs.mk

TEST_OBJS = $(OBJ_DIR)/test_libeprom24x.o

# ----- Targets

.PHONY : app_clean

$(TEST_APP_NAME) : $(TEST_OBJS) $(LIB_NAME)
	$(CC) $(LINK_FLAGS) -o $(TEST_APP_NAME) $(TEST_OBJS) $(LIB_DIRS) $(LIBS)

app_clean :
	rm -f $(TEST_OBJS) $(SIM_OBJS) $(OBJ_DIR)/*.$(ARCH) *~
