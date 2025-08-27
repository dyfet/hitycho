# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 David Sugar <tychosoft@gmail.com>

cmake_minimum_required(VERSION 3.16.0)
project(HiTycho VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

file(
    GLOB LINT_SOURCES
    RELATIVE ${PROJECT_SOURCE_DIR}
    src/*.hpp test/*.cpp)

include(cmake/custom.cmake OPTIONAL)
include(cmake/project.cmake)
include(cmake/features.cmake)
include(cmake/coverage.cmake)
include(cmake/linting.cmake)
include(cmake/deploy.cmake OPTIONAL)

file(GLOB optional .git[a-z]* *.json *.in *.yml .clang* cmake/*)
file(GLOB markdown *.md)
file(GLOB headers src/*.hpp)

# Setup headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
configure_file(hitycho.pc.in hitycho.pc)

# Testing
enable_testing()

add_executable(test_atomic test/atomic.cpp src/common.hpp src/atomic.hpp)
add_test(NAME test-atomic COMMAND test_atomic)
target_include_directories(test_atomic PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_atomic PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_binary test/binary.cpp src/common.hpp src/binary.hpp)
add_test(NAME test-binary COMMAND test_binary)
target_include_directories(test_binary PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_binary PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_expected test/expected.cpp src/common.hpp src/expected.hpp)
add_test(NAME test-expected COMMAND test_expected)
target_include_directories(test_expected PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_expected PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_locking test/locking.cpp src/common.hpp src/locking.hpp)
add_test(NAME test-locking COMMAND test_locking)
target_include_directories(test_locking PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_locking PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_scan test/scan.cpp src/common.hpp src/scan.hpp)
add_test(NAME test-scan COMMAND test_scan)
target_include_directories(test_scan PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_scan PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_strings test/strings.cpp src/common.hpp src/strings.hpp)
add_test(NAME test-strings COMMAND test_strings)
target_include_directories(test_strings PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_strings PRIVATE ${HPX_APPLICATION_LIBRARIES} HPX::hpx HPX::wrap_main)

add_executable(test_print test/print.cpp src/common.hpp src/print.hpp)
add_test(NAME test-print COMMAND test_print)
target_include_directories(test_print PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_print PRIVATE HPX::hpx HPX::wrap_main HPX::iostreams_component)

add_executable(test_sockets test/sockets.cpp src/sockets.hpp)
add_test(NAME test-sockets COMMAND test_sockets)
target_include_directories(test_sockets PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_sockets PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_sync test/sync.cpp src/common.hpp src/sync.hpp)
add_test(NAME test-sync COMMAND test_sync)
target_include_directories(test_sync PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_sync PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_timer test/timer.cpp src/common.hpp src/timer.hpp)
add_test(NAME test-timer COMMAND test_timer)
target_include_directories(test_timer PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_timer PRIVATE HPX::hpx HPX::wrap_main)

add_executable(test_final test/finalize.cpp src/common.hpp src/finalize.hpp)
add_test(NAME test-final COMMAND test_final)
target_include_directories(test_final PRIVATE ${HPX_APPLICATION_INCLUDE_DIRS})
target_link_libraries(test_final PRIVATE HPX::hpx HPX::wrap_main)

# Extras...
add_custom_target(header-files SOURCES ${headers})
add_custom_target(support-files SOURCES ${markdown} ${optional})

install(FILES ${headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/hitycho")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hitycho.pc DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
