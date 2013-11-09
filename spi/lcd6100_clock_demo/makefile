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

.PHONY : all clean

app :
	cd ./src ; make $(APP_NAME)

all : app

clean :
	rm -f *~
	cd ./src ; make app_clean
