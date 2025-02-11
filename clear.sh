#!/bin/bash

CURDIR=$(dirname $0);
echo -e "\033[0;32mClean: $(readlink -f $CURDIR) \033[0m";

rm -rf \
  ${CURDIR:?}/bin/ \
  ${CURDIR:?}/lib/ \
  ${CURDIR:?}/build/ \
  ${CURDIR:?}/doc/latex/ \
  ${CURDIR:?}/doc/html/ \
  ${CURDIR:?}/CMakeCache.txt \
  ${CURDIR:?}/CMakeFiles/ \
  ${CURDIR:?}/cmake_install.cmake \
  ${CURDIR:?}/Makefile \
  ${CURDIR:?}/source/*/*/CMakeFiles/ \
  ${CURDIR:?}/source/*/*/cmake_install.cmake \
  ${CURDIR:?}/source/*/*/Makefile

if [ "$#" -gt "0" ] 
then 
  if [ "$1" == "glfw" ] || [ "$1" == "all" ]
  then 
    echo -e "\033[0;32mClean: ${CURDIR}/external/glfw \033[0m";
    rm -rf ${CURDIR}/external/glfw;
  fi
  if [ "$1" == "glm" ] || [ "$1" == "all" ]
  then 
    echo -e "\033[0;32mClean: ${CURDIR}/external/glm \033[0m";
    rm -rf ${CURDIR}/external/glm;
  fi
fi
