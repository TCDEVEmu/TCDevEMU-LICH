#!/bin/bash

set -e

cat >>conf/config.sh <<CONFIG_SH
MTHREADS=$(($(grep -c ^processor /proc/cpuinfo) + 2))
CAPPS_BUILD=none
CTOOLS_BUILD=maps-only
CSCRIPTPCH=OFF
CCOREPCH=OFF
CCUSTOMOPTIONS='-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_FLAGS="-Werror" -DCMAKE_CXX_FLAGS="-Werror"'
CONFIG_SH

case $COMPILER in

  # this is in order to use the "default" clang version of the OS, without forcing a specific version
  "clang" )
    time sudo apt-get install -y clang
    echo "CCOMPILERC=\"clang\"" >> ./conf/config.sh
    echo "CCOMPILERCXX=\"clang++\"" >> ./conf/config.sh
    ;;

  * )
    echo "Unknown compiler $COMPILER"
    exit 1
    ;;
esac
