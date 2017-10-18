cmake_minimum_required(VERSION 3.8)
project(MCF_software_package)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        FDM.cpp
        fdm.h)

add_executable(MCF_software_package ${SOURCE_FILES})