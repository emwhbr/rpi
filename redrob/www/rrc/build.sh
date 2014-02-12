#!/bin/bash
# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

CLASS_DIR="./class"
DEPLOY_DIR="./deploy"
DEPLOY_JAR_DIR="./deploy_jar"
HTML_DIR="./html"
JAVA_DIR="./java"
RES_DIR="./res"
SIM_DIR="./sim"

case "$1" in
    applet)
        echo "==[APPLET]==="
	javac -cp ${CLASS_DIR} \
              -d ${CLASS_DIR} \
	      -sourcepath ${JAVA_DIR} \
              ${JAVA_DIR}/*.java
        ;;

    sim)
	echo "==[SIMULATOR]==="
	javac -cp ${SIM_DIR}/class \
              -d ${SIM_DIR}/class \
	      -sourcepath ${SIM_DIR}/java \
              ${SIM_DIR}/java/*.java
	;;

    deploy)
	echo "==[DEPLOY]==="
	cp -r ${CLASS_DIR}/*      ${DEPLOY_DIR}
	cp ${HTML_DIR}/index.html ${DEPLOY_DIR}
	cp ${RES_DIR}/*.png       ${DEPLOY_DIR}	
	;;

    deploy_jar)
	echo "==[DEPLOY-JAR]==="
	cp -r ${CLASS_DIR}/* ${DEPLOY_JAR_DIR}
	cp ${RES_DIR}/*.png  ${DEPLOY_JAR_DIR}
	jar cvf0e /tmp/rrc.jar AppletRrc -C ${DEPLOY_JAR_DIR} .
	rm -rf ${DEPLOY_JAR_DIR}/*
	mv /tmp/rrc.jar ${DEPLOY_JAR_DIR}/
	cp ${HTML_DIR}/index_jar.html ${DEPLOY_JAR_DIR}/index.html	
	;;

    clean)
        echo "==[CLEAN]==="
	rm -rf ./*~        	
	rm -rf ${CLASS_DIR}/*
	rm -rf ${DEPLOY_DIR}/*
	rm -rf ${DEPLOY_JAR_DIR}/*
	rm -rf ${HTML_DIR}/*~
	rm -rf ${JAVA_DIR}/*~
	rm -rf ${RES_DIR}/*~
	rm -rf ${SIM_DIR}/*~
	rm -rf ${SIM_DIR}/java/*~
	rm -rf ${SIM_DIR}/class/*
        ;;

    *)
        echo "Usage $0 {applet|sim|deploy|deploy_jar|clean}"
        exit 1
        ;;
esac
