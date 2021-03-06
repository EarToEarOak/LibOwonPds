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


if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "/MT /O2 /Ob2 /D NDEBUG")
endif()

# Static library
add_library(libowonpds_static STATIC
    libowonpds.c
    libowonpds_helper.c)
target_link_libraries(libowonpds_static
    ${LIBUSB_LIBRARY}
    ${PNG_LIBRARIES})
set_target_properties(libowonpds_static PROPERTIES
    OUTPUT_NAME owonpds)

# Shared library
add_library(libowonpds_shared SHARED
    libowonpds.c
    libowonpds_helper.c)
if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(libowonpds_shared
        "-Wl,--whole-archive"
        ${LIBUSB_LIBRARY}
        ${PNG_LIBRARIES}
        "-Wl,--no-whole-archive")
else()
    target_link_libraries(libowonpds_shared
        ${LIBUSB_LIBRARY}
        ${PNG_LIBRARIES})
endif()
set_target_properties(libowonpds_shared PROPERTIES
    OUTPUT_NAME owonpds)

# Utility
add_executable(owonpds
    owonpds.c)
target_link_libraries(owonpds
    libowonpds_static
    ${LIBUSB_LIBRARY}
    ${PNG_LIBRARIES})

# Install targets
install(
    TARGETS
        libowonpds_static
        libowonpds_shared
        owonpds
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin)
