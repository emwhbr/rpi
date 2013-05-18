# /*************************************************************
# *                                                            *
# * Copyright (C) Bonden i Nol                                 *
# *                                                            *
# **************************************************************/

include common_defs.mk

# ------ Targets

.PHONY : libeprom_24x test_libeprom_24x all clean

libeprom_24x :
	cd ./src ; make $(LIB_NAME)

test_libeprom_24x :
	cd ./test ; make $(TEST_APP_NAME)

all : libeprom_24x test_libeprom_24x

clean :
	rm -f *~
	cd ./src ; make lib_clean
	cd ./test ; make app_clean
