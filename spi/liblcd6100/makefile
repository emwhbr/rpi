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

.PHONY : liblcd6100 test_liblcd6100 all clean

liblcd6100 :
	cd ./src ; make $(LIB_NAME)

test_liblcd6100 :
	cd ./test ; make $(TEST_APP_NAME)

all : liblcd6100 test_liblcd6100

clean :
	rm -f *~
	cd ./src ; make lib_clean
	cd ./test ; make app_clean
