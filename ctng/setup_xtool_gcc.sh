#!/bin/bash

ARCH=arm
CROSS_COMPILE=arm-unknown-linux-gnueabihf-

# The toolchain
PATH=/opt/rpi/toolchains/x-tools/arm-unknown-linux-gnueabihf/bin:$PATH

export ARCH
export CROSS_COMPILE
export PATH
