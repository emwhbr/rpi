#!/bin/bash

# Clean
rm *.o *.arm *~

# Build
arm-unknown-linux-gnueabihf-gcc -c 24cXX.c
arm-unknown-linux-gnueabihf-gcc -o eeprog.arm eeprog.c 24cXX.o
