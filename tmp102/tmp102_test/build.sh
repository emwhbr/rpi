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

case "$1" in
    release)
        echo "==[MAKE RELEASE]==="
        make BUILD_TYPE=RELEASE all
        ;;

    debug)
        echo "==[MAKE DEBUG]==="
        make BUILD_TYPE=DEBUG all
        ;;

    clean)
        echo "==[CLEANUP]==="
        make clean
        ;;

    *)
        echo "Usage $0 {release|debug|clean}"
        exit 1
        ;;
esac
