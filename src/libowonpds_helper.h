/*
 * LibOwonPds
 *
 * A userspace driver of Owon PDS oscilloscopes
 *
 * http://eartoearoak.com/software/libowonpds
 *
 * Copyright 2015 Al Brown
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @file
 * @defgroup 	LibOwonPdsHelper
 * @{
 * @brief		Helper functions for LibOwonPds
 * @author		Al Brown
 * @copyright	Copyright &copy; 2015 Al Brown
 *
 */

#ifndef LIBOWONPDS_HELPER_H_
#define LIBOWONPDS_HELPER_H_

#include <stdbool.h>

#include "libowonpds.h"

int owon_write_vector_csv(const OWON_SCOPE_T *scope, const char* filename,
		const bool verbose);
int owon_write_bitmap_png(const OWON_SCOPE_T *scope, const char* filename);

#endif /* LIBOWONPDS_HELPER_H_ */

/** @}*/
