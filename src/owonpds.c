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

#include <libusb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libowonpds.h"
#include "libowonpds_helper.h"

#define EXT_CSV ".csv"
#define EXT_PNG ".png"

#ifndef VERSION
#define VERSION ""
#endif

/**
 * Get data from a scope
 *
 * Retrieves information from the scope.\n
 * If a filename is given (without an extension) the captured data will
 * be saved to either a CSV or PNG file.
 */

int main(int argc, char *argv[]) {

	OWON_SCOPE_T scope;
	int error_code;

	fprintf(stdout, "owonpds utility %s\n\n", VERSION);

	error_code = owon_open(&scope, 0);
	if (error_code == LIBUSB_SUCCESS) {
		error_code = owon_read(&scope);

		if (scope.type == OWON_CHANNEL) {
			fprintf(stdout, "Channel Data\n");
			fprintf(stdout, "Name        %s\n", scope.name);

			unsigned i;
			OWON_CHANNEL_T channel;
			for (i = 0; i < scope.channelCount; i++) {
				channel = scope.channel[i];
				fprintf(stdout, "  Channel     %s\n", channel.name);
				fprintf(stdout, "    Timebase    %.9fs\n", channel.timebase);
				fprintf(stdout, "    Position    %.3fs\n", channel.slow);
				fprintf(stdout, "    Offset      %.3fv\n", channel.offset);
				fprintf(stdout, "    Sensitivity %.2fv\n", channel.sensitivity);
				fprintf(stdout, "    Attenuation %uX\n", channel.attenuation);
			}
		} else
			fprintf(stdout, "Bitmap Data\n");
	}

	if (argc == 2) {
		size_t length = strlen(argv[1]);
		char *filename = malloc(length + 5);
		if (filename){
			memcpy(filename, argv[1], length);
			if (scope.type == OWON_CHANNEL) {
				memcpy(&filename[length], EXT_CSV, sizeof(EXT_CSV));
				owon_write_vector_csv(&scope, filename, true);
			}
			else {
				memcpy(&filename[length], EXT_PNG, sizeof(EXT_PNG));
				owon_write_bitmap_png(&scope, filename);
			}
		}
	}

	owon_close(&scope);

	return (error_code);
}

