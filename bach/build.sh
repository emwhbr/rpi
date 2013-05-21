#!/bin/bash
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

show_usage () {
   echo "Usage $0 {driver <path_to_kernel>|app|clean}" 
}

case "$1" in
    driver)
        echo "==[MAKE DRIVER]==="
	if [ ! -d "$2" ]; then
	  show_usage
          exit 1  
	fi
        make driver KDIR=$2
        ;;

    app)
        echo "==[MAKE APPLICATION]==="
        make app
        ;;

    clean)
        echo "==[CLEANUP]==="
        make clean
        ;;

    *)
	show_usage
        exit 1
        ;;
esac
