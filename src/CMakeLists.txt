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

# Library
add_library(libowonpds STATIC
    libowonpds.c
    libowonpds_helper.c)
set_target_properties(libowonpds PROPERTIES
    OUTPUT_NAME owonpds)

# Utility
add_executable(owonpds
    owonpds.c)
target_link_libraries(owonpds
    libowonpds
    ${LIBUSB_LIBRARY}
    ${PNG_LIBRARIES})

# Install targets
install(TARGETS libowonpds owonpds
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
)