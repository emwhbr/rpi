# /***********************************************************************
# *                                                                      *
# * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
# *                                                                      *
# * This program is free software; you can redistribute it and/or modify *
# * it under the terms of the GNU General Public License as published by *
# * the Free Software Foundation; either version 2 of the License, or    *
# * (at your option) any later version.                                  *
# *                                                                      *
# * Makefile for BACH kernel driver and user space test application.     *
# *                                                                      *
# ************************************************************************/

# Common
ARCH          := arm
CROSS_COMPILE := arm-unknown-linux-gnueabihf-

# Kernel driver
MODULE := bach
obj-m  := $(MODULE).o
PWD    := $(shell pwd)

# User space application
APP_NAME := bach_test

# Available make targets
all:	clean driver app

driver:
ifndef KDIR
	@echo "*** ERROR : Missing argument KDIR (path to kernel sources)"
	exit 1
endif
	$(MAKE) -C $(KDIR) M=$(PWD) modules

app:
	$(CROSS_COMPILE)gcc -o $(APP_NAME).arm $(APP_NAME).c

clean:
	rm -rf *.o *.ko *~
	rm -rf .tmp_versions .*.cmd .*.mod.o.d
	rm -rf *.mod.c modules.order Module.symvers
	rm -rf $(APP_NAME).arm

help:
	@echo "Usage: make clean"
	@echo "       make driver KDIR=<path_to_kernel_sources>"
	@echo "       mapp app"
