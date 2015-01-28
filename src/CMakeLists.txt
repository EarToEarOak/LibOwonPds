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

# Shared library
add_library(libowonpds_shared SHARED
	libowonpds.c
	libowonpds_helper.c)
target_link_libraries(libowonpds_shared
	${USB_LIBRARIES}
	${PNG_LIBRARIES})
set_target_properties(libowonpds_shared PROPERTIES
    OUTPUT_NAME owonpds)

# Static library
add_library(libowonpds_static STATIC
    libowonpds.c
    libowonpds_helper.c)
target_link_libraries(libowonpds_static
    ${USB_LIBRARIES})
set_target_properties(libowonpds_static PROPERTIES
    OUTPUT_NAME owonpds)

# Utility
add_executable(owonpds
    owonpds.c)
target_link_libraries(owonpds
    libowonpds_shared
    ${PNG_LIBRARIES})

set(INSTALL_TARGETS libowonpds_shared libowonpds_static owonpds)