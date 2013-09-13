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

.PHONY : libraspi test_libraspi all clean

libraspi :
	cd ./src ; make $(LIB_NAME)

test_libraspi :
	cd ./test ; make $(TEST_APP_NAME)

all : libraspi test_libraspi

clean :
	rm -f *~
	cd ./src ; make lib_clean
	cd ./test ; make app_clean
