#!/bin/bash

### Toolchain setup
PATH=/opt/rpi/toolchains/x-tools/arm-unknown-linux-gnueabihf/bin:$PATH

### Clean
rm *.o *.arm *~

### Build
arm-unknown-linux-gnueabihf-gcc -c 24cXX.c
arm-unknown-linux-gnueabihf-gcc -o eeprog.arm eeprog.c 24cXX.o
