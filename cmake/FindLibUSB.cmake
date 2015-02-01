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

# Look in the standard places
find_path(LIBUSB_INCLUDE_DIR
    NAMES libusb.h
    PATHS
        /usr/include
        /usr/local/include
    PATH_SUFFIXES
        libusb-1.0)
find_library(LIBUSB_LIBRARY
    NAMES
        libusb-1.0.so
        libusb-1.0.a
        libusb-1.0.lib
        libusb-1.0.dll
        libusb-1.0.dll.a
    PATHS
        /lib
        /usr/lib
        /usr/local/lib)

# Extract version
if(LIBUSB_INCLUDE_DIR)
    if(EXISTS "${LIBUSB_INCLUDE_DIR}/version.h")
        file(STRINGS "${LIBUSB_INCLUDE_DIR}/version.h"
            VERSION_H
            REGEX "^#define LIBUSB_M.+ [0-9]+$")

        if("${VERSION_H}" MATCHES "#define LIBUSB_MAJOR ([0-9]+)")
            set(LIBUSB_VERSION_MAJOR ${CMAKE_MATCH_1})
        endif()
        if("${VERSION_H}" MATCHES "#define LIBUSB_MINOR ([0-9]+)")
            set(LIBUSB_VERSION_MINOR ${CMAKE_MATCH_1})
        endif()
        if("${VERSION_H}" MATCHES "#define LIBUSB_MICRO ([0-9]+)")
            set(LIBUSB_VERSION_MICRO ${CMAKE_MATCH_1})
        endif()
        set(LIBUSB_VERSION ${LIBUSB_VERSION_MAJOR}.${LIBUSB_VERSION_MINOR}.${LIBUSB_VERSION_MICRO})
    elseif(EXISTS "${LIBUSB_INCLUDE_DIR}/libusb.h")
        file(STRINGS "${LIBUSB_INCLUDE_DIR}/libusb.h"
            LIBUSB_H
            REGEX "^#define LIBUSB[X]?_API_VERSION 0x.+$")
        if("${LIBUSB_H}" MATCHES "#define LIBUSB_API_VERSION (0x.+)")
            set(LIBUSB_VERSION ${CMAKE_MATCH_1})
         elseif("${LIBUSB_H}" MATCHES "#define LIBUSBX_API_VERSION (0x.+)")
             set(LIBUSB_VERSION ${CMAKE_MATCH_1})
        endif()
    endif()


endif()

# Test variables
find_package_handle_standard_args(LibUSB
    REQUIRED_VARS
        LIBUSB_INCLUDE_DIR
        LIBUSB_LIBRARY
        LIBUSB_VERSION
    VERSION_VAR
        LIBUSB_VERSION
)
mark_as_advanced(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARY)


