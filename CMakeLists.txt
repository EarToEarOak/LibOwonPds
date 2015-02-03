#
# LibOwonPds
#
# A userspace driver of Owon PDS oscilloscopes
#
# http://eartoearoak.com/software/libowonpds
#
# Copyright 2015 Al Brown
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(THREADS_USE_PTHREADS_WIN32 true)

# Compiler
if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-fPIC)
    add_definitions(-std=c99)
    add_definitions(-Wall)
    add_definitions(-Wextra)
    add_definitions(-Wconversion)
    add_definitions(-Wno-unused-parameter)
    add_definitions(-Wno-unused)
    add_definitions(-Wsign-compare)
    add_definitions(-Wdeclaration-after-statement)
    add_definitions(-fvisibility=hidden)
 endif()

# Project
project(LibOwonPds C)

# Dependencies
find_package(Git)
find_package(LibUSB REQUIRED)
find_package(PNG REQUIRED)
find_package(PythonLibs)

if(NOT GIT_FOUND)
    message(STATUS "Git not found - versioning disabled")
endif()

if(NOT PYTHONLIBS_FOUND)
    message(STATUS "Python not found - support disabled")
endif()

# Version
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(LibOwonPds_VERSION_MAJOR 1)
    set(LibOwonPds_VERSION_MINOR 0)
    set(LibOwonPds_VERSION_PATCH ${GIT_HASH})
    set(VERSION ${LibOwonPds_VERSION_MAJOR}.${LibOwonPds_VERSION_MINOR}.${LibOwonPds_VERSION_PATCH})
    add_definitions(-DVERSION="${VERSION}")
endif()

# udev rules
if(UNIX)
    option(INSTALL_UDEV "Install udev rules" ON)
    if (INSTALL_UDEV)
        install (
            FILES owonpds.rules
            DESTINATION "/etc/udev/rules.d")
        message (STATUS "Installing udev rules")
    else()
        message (STATUS "Not installing udev rules")
    endif()
endif()

# Python
if(PYTHONLIBS_FOUND)
    option(INSTALL_PYTHON "Python support" ON)
endif()

# Directories
include_directories(
    ${LIBUSB_INCLUDE_DIR}
    ${PNG_PNG_INCLUDE_DIR})
link_directories(
    ${LIBUSB_LIBRARY}
    ${PNG_LIBRARY})
add_subdirectory(src)
add_subdirectory(src/python)


# Uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Print info
MESSAGE(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Version: ${VERSION}")
