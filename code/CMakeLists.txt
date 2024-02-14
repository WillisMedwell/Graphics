cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable unity builds
set(CMAKE_UNITY_BUILD true)

# Enabling LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT has_ipo_support)
if(has_ipo_support)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG -ftime-trace)

project(Graphics)

add_subdirectory(Engine)
add_subdirectory(Demos)

add_subdirectory(Test)

