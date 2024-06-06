#!/bin/bash
shopt -s extglob
rm -r !("build.sh"|"clean.sh"|"CMakeLists.txt"|"main.cpp"|"KeyBasedSample.cpp"|"KeyBasedSample.h"|"UserBasedSample.cpp"|"UserBasedSample.h"|"SampleBase.cpp"|"SampleBase.h"|"CppSample.xcodeproj")
