# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(FindPkgConfig)

if(WIN32)
    message(FATAL_ERROR "HPX may support windows but this library does not.")
endif()

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(HPX REQUIRED)
pkg_check_modules(HPX_APPLICATION REQUIRED hpx_application)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(BUILD_DEBUG true)
    add_compile_definitions(DEBUG=1)
else()
    add_compile_definitions(NDEBUG=1)
endif()
