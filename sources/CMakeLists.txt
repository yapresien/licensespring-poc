cmake_minimum_required(VERSION 3.10)

project(presien-lic-app)

if(WIN32 AND (NOT MINGW) AND (NOT MSYS))
    include(CMakeListsWindows.txt)
else()
    include(CMakeListsUnix.txt)
endif()
