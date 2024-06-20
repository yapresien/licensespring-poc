#!/bin/bash

unameOut="$(uname -s)"
case "${unameOut}" in
Linux*)     cproc=$(nproc);;
Darwin*)    cproc=$(sysctl -n hw.logicalcpu);;
*)          cproc="1"
esac
echo ${machine}

exit_bad_arguments()
{
  echo "no arguments defaulting debug static"
#   echo "usage: $0 {release|debug} {static|shared}"
#   exit 1
    BUILD_TYPE_CHECK=release
    LINK_TYPE_CHECK=static

}
if [ "$#" -ge 2 ]; then
    if [ ${BUILD_TYPE_CHECK,,} != "release" ] && [ ${BUILD_TYPE_CHECK,,} != "debug" ]; then
        exit_bad_arguments
    fi
    if [ ${LINK_TYPE_CHECK,,} != "static" ] && [ ${LINK_TYPE_CHECK,,} != "shared" ]; then
        exit_bad_arguments
    fi
else
    exit_bad_arguments
fi

SHARED_FLAG="ON"
if [ ${LINK_TYPE_CHECK,,} == "static" ]; then
    SHARED_FLAG="OFF"
fi

BUILD_TYPE_FLAG="Release"
if [ ${BUILD_TYPE_CHECK,,} == "debug" ]; then
    BUILD_TYPE_FLAG="Debug"
fi

OUTPUT_PATH="${BUILD_TYPE_FLAG}_${LINK_TYPE_CHECK}"
OUTPUT_PATH=${OUTPUT_PATH,,}

if [ -d ${OUTPUT_PATH} ]; then
    rm -r ${OUTPUT_PATH}
fi

mkdir ${OUTPUT_PATH}
cd ${OUTPUT_PATH}

BUILD_COMPILER=$(echo -e 'all:\n\techo $(CC)\n' | make -sf -)
CMAKE_GEN="Unix Makefiles"

cmake_cmd="cmake"

if expr "${BUILD_COMPILER}" : ".*mingw.*"; then
    DYLIB_EXTENSION="dll"
    CMAKE_GEN="MinGW Makefiles"
    [ ! -z "${CMAKE_PATH}" ] && [ -x "${CMAKE_PATH}" ] && cmake_cmd="${CMAKE_PATH}"
fi

$cmake_cmd -G "${CMAKE_GEN}" -DCMAKE_BUILD_TYPE=${BUILD_TYPE_FLAG} -DUSE_SHARED_LIBS=${SHARED_FLAG} -DCMAKE_COLOR_MAKEFILE=ON ..
$cmake_cmd --build . --target all
