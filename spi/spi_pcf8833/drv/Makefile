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

# ----- Kernel driver

MODULE = spi-pcf8833
obj-m  = $(MODULE).o
PWD    = $(shell pwd)

# ----- Available make targets

.PHONY : check-env

driver : check-env
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean : check-env
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *~

check-env:
ifndef KDIR
	@echo "*** ERROR : Missing argument KDIR (path to kernel sources)"
	exit 1
endif
ifndef ARCH
	@echo "*** ERROR : Missing argument ARCH"
	exit 1
endif
ifndef CROSS_COMPILE
	@echo "*** ERROR : Missing argument CROSS_COMPILE"
	exit 1
endif
