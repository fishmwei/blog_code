#!/bin/sh

FILENAME=./version.h

echo '#ifndef _VERSION_H_'>$FILENAME
echo '#define _VERSION_H_'>>$FILENAME


build_date=`date +"%k:%M:%S %m-%d-%Y"`



echo "#define BUILD_DATE  \"${build_date}\"">>$FILENAME
echo "#define BUILD_LINUX_USER \"${USER}\"">>$FILENAME


echo '#endif'>>$FILENAME
