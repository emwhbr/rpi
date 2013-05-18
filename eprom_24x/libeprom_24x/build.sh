#!/bin/bash
# /*************************************************************
# *                                                            *
# * Copyright (C) Bonden i Nol                                 *
# *                                                            *
# **************************************************************/

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
