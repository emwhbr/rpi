#!/bin/bash
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

################################################################
function get_parallel_args()
################################################################
{
    # Check number of CPU's in this machine
    nr_cpus=`cat /proc/cpuinfo | grep processor | wc -l`
    
    # Add one to get number of parallel jobs
    ((nr_jobs=nr_cpus + 1))
    
    echo "-j${nr_jobs}"
    return 0
}

### Number of parallel jobs on this machine
PARALLEL_ARGS=`get_parallel_args`

### Toolchain setup
PATH=/opt/rpi/toolchains/x-tools/arm-unknown-linux-gnueabihf/bin:$PATH

### Kernel sources
KDIR=/proj/rpi/kernel/linux-rpi-3.6.y

case "$1" in
    all)
        echo "==[MAKE ALL]==="
        make JOBS=${PARALLEL_ARGS} all KDIR=${KDIR}
        ;;

    clean)
        echo "==[CLEANUP]==="
        make clean KDIR=${KDIR}
        ;;

    *)
        echo "Usage $0 {all|clean}"
        exit 1
        ;;
esac
