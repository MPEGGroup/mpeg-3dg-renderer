#!/bin/bash

CURDIR=$(dirname $0);
echo -e "\033[0;32mCmake: $(readlink -f $CURDIR) \033[0m";

CMAKE="";
if [ "$( cmake  --version 2>&1 | grep version | awk '{print $3 }' | awk -F '.' '{print $1}' )" == 3 ] ; then CMAKE=cmake; fi
if [ "$( cmake3 --version 2>&1 | grep version | awk '{print $3 }' | awk -F '.' '{print $1}' )" == 3 ] ; then CMAKE=cmake3; fi
if [ "$CMAKE" == "" ] ; then echo "Can't find cmake > 3.0"; exit; fi

case "$(uname -s)" in
  Linux*)     MACHINE=Linux;;
  Darwin*)    MACHINE=Mac;;
  CYGWIN*)    MACHINE=Cygwin;;
  MINGW*)     MACHINE=MinGw;;
  *)          MACHINE="UNKNOWN"
esac

MODE=Release;
FORMAT=0;
TIDY=0;
CMAKE_FLAGS=;
USE_OPENMP=ON;
if [ "$MACHINE" == "Linux" ] ; then NUMBER_OF_PROCESSORS=$(grep -c ^processor /proc/cpuinfo); fi

for i in "$@"
do  
  case "$i" in
    debug)       MODE=Debug;;
    release)     MODE=Release;;
    format)      FORMAT=1;;
    tidy)        TIDY=1;;
    noomp|NOOMP) USE_OPENMP=OFF;;
    *)       echo "ERROR: arguments \"$i\" not supported: option = [debug|release]"; exit -1;;
  esac
done

CMAKE_FLAGS="$CMAKE_FLAGS -DCMAKE_CONFIGURATION_TYPES=$MODE -DUSE_OPENMP=$USE_OPENMP";
$CMAKE -H${CURDIR} -B${CURDIR}/build/${MODE} ${CMAKE_FLAGS}

if [ $FORMAT == 1 ] 
then 
  echo -e "\033[0;32mFormat: $(readlink -f $CURDIR) \033[0m";
  case "${MACHINE}" in
    Linux) $CMAKE --build ${CURDIR}/build/${MODE} --target clang-format;; 
    Mac)   echo "Please, open the generated xcode project and build it ";;
    *)     buildWindows ./build/${MODE}/clang-format.vcxproj  ${MODE};;
  esac 
  echo -e "\033[0;32mdone \033[0m";
  exit;
fi 

if [ $TIDY == 1 ] 
then 
  echo -e "\033[0;32mTidy: $(readlink -f $CURDIR) \033[0m";
  case "${MACHINE}" in
    Linux) $CMAKE --build ${CURDIR}/build/${MODE} --target clang-tidy;; 
    Mac)   echo "Please, open the generated xcode project and build it ";;
    *)     buildWindows ./build/${MODE}/clang-tidy.vcxproj ${MODE};;
  esac 
  echo -e "\033[0;32mdone \033[0m";
  exit;
fi 

echo -e "\033[0;32mBuild: $(readlink -f $CURDIR) \033[0m";
${CMAKE} --build ${CURDIR}/build/${MODE} --config ${MODE} --parallel ${NUMBER_OF_PROCESSORS}
if [[ $? -ne 0 ]] ; then exit 1; fi 
echo -e "\033[0;32mdone \033[0m";
