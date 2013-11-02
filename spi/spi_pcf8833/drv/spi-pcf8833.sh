#!/bin/sh
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

###########################################################################
#
# Description:
# Utility script for load/unload of kernel module 'spi_pcf8833' and  
# management of module device files. The module uses dynamic allocation to
# retreive major device number.
#
# Note!
# This script assumes that the RFS is mounted RW to allow the creation of
# device files in directory '/dev'.
#
###########################################################################


module_name="spi-pcf8833"
device_name=${module_name}

################################################################
function print_usage_and_die()
################################################################
{
    echo "Usage: `basename $0` load <path_to_ko_file>"
    echo "       `basename $0` unload"
    exit 1
}

################################################################
function load_module()
################################################################
{
    echo "LOAD: $1"

    # Check expected .ko file
    if [ "`basename $1`" != "${module_name}.ko" ]; then
	echo "*** ERROR: expected ${module_name}.ko"
	exit 1
    fi

    # Load kernel module
    /sbin/insmod $1
    if [ $? -ne 0 ]; then
	echo "*** ERROR: failed to load module ${module_name}"
	exit 1
    fi

    # Get major device number for kernel module
    major=$(awk "\$2==\"${module}\" {print \$1}" /proc/devices)

    echo "LOAD: major device number = $major"
    
    # Remove old device files
    rm -rf /dev/${device}*
    
    # Create new device files
    mknod /dev/${device}-0 c ${major} 0
    mknod /dev/${device}-1 c ${major} 1
}

################################################################
function unload_module()
################################################################
{
    echo "UNLOAD: ${module_name}"

    # Unload kernel module
    rmmod ${module_name}
    if [ $? -ne 0 ]; then
	echo "*** ERROR: failed to unload module ${module_name}"
	exit 1
    fi

    # Remove device files
    rm -rf /dev/${device}*
}

################################################################
#                Main script starts here
################################################################
case "$1" in
    load)
	if [ $# -ne 2 ]; then
	  print_usage_and_die  
	fi
	load_module $2
        ;;

    unload)
	if [ $# -ne 1 ]; then
	  print_usage_and_die  
	fi
	unload_module
        ;;

    *)
	print_usage_and_die
        ;;
esac
