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
