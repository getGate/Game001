cmake_minimum_required(VERSION 3.0.0)
project(Game001 VERSION 0.1.0)

include(cmake/utils.cmake)

enable_language(C CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(GAME_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

SET(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SUBDIRLIST(TARGET_DIRS ${TARGET_DIR})
foreach(dir ${TARGET_DIRS})
    if(IS_DIRECTORY "${TARGET_DIR}/${dir}")
        message("add sub dir: " ${dir})
        add_subdirectory("src/${dir}")
    endif(IS_DIRECTORY "${TARGET_DIR}/${dir}")
endforeach()