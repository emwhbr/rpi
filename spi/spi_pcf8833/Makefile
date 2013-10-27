# /***********************************************************************
# *                                                                      *
# * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
# *                                                                      *
# * This program is free software; you can redistribute it and/or modify *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation; either version 2 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# ************************************************************************/

include ./common_defs.mk

# ----- Available make targets

.PHONY : driver test clean

all : driver test

driver :
	cd ./drv ; make driver KDIR=$(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

test :
	cd ./test ; make test

clean :
	rm -f *~
	cd ./drv ;  make clean KDIR=$(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
	cd ./test ; make clean
