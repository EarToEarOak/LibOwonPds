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
 */

#include <png.h>
#include <pngconf.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "libowonpds.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void png_close(png_structp *png, FILE *file) {

	png_destroy_write_struct(png, (png_infopp) NULL);
	fclose(file);

}

/**
 * Write channel data to a CSV file
 *
 * @param scope		Scope structure
 * @param filename	Filename
 * @param verbose 	Include scope information
 *
 * @return 0 Success, >0 OWON_ERROR error, <0 errno error
 *
 */
LIBOWONPDS_EXPORT int owon_write_csv(const OWON_SCOPE_T *scope,
		const char* filename, const bool verbose) {

	if (scope->type != OWON_VECTOR)
		return (OWON_ERROR_FORMAT);

	FILE *file;

	errno = 0;
	file = fopen(filename, "w");
	if (!file)
		return errno;

	if (verbose) {
		fprintf(file, "Device, %s\n", scope->name);
		fprintf(file, "Active Channels, %u\n", scope->channel_count);
	}

	uint32_t max_len = 0;
	unsigned i;
	for (i = 0; i < scope->channel_count; i++) {
		max_len = MAX(max_len, scope->channel[i].samples);
		fprintf(file, "CH%u Time (s), CH%u Level (V)", i + 1, i + 1);
		if (i < scope->channel_count - 1)
			fprintf(file, ", ");
	}
	fprintf(file, "\n");

	unsigned j;
	for (i = 0; i < max_len; i++) {
		for (j = 0; j < scope->channel_count; j++) {
			OWON_CHANNEL_T channel = scope->channel[j];
			double time = i / channel.sample_rate;
			if (i < channel.samples) {
				fprintf(file, "%.12f, %f", time, channel.vector[i]);
				if (j < scope->channel_count - 1)
					fprintf(file, ", ");
			}
		}
		fprintf(file, "\n");
	}

	fclose(file);

	return errno;
}

/**
 * Write bitmap data to a PNG file
 *
 * @param scope		Scope structure
 * @param filename	Filename
 *
 * @return 0 Success, >0 OWON_ERROR error, <0 errno error
 *
 */
LIBOWONPDS_EXPORT int owon_write_png(const OWON_SCOPE_T *scope,
		const char* filename) {

	if (scope->type != OWON_BITMAP)
		return (OWON_ERROR_FORMAT);

	FILE *file;
	errno = 0;
	file = fopen(filename, "w");
	if (!file)
		return errno;

	png_structp png;
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png) {
		png_infop info = png_create_info_struct(png);
		if (info) {
			if (setjmp(png_jmpbuf(png))) {
				png_close(&png, file);
				return (OWON_ERROR_PNG);
			}
			png_init_io(png, file);

			png_set_IHDR(png, info, OWON_BITMAP_WIDTH, OWON_BITMAP_HEIGHT, 8,
			PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_write_info(png, info);

			png_byte *rows[OWON_BITMAP_HEIGHT];
			int rowSize = OWON_BITMAP_WIDTH * sizeof(char)
					* OWON_BITMAP_CHANNELS;
			int i;
			for (i = 0; i < OWON_BITMAP_HEIGHT; i++) {
				rows[i] = &scope->bitmap[i * rowSize];
			}
			png_set_bgr(png);
			png_write_image(png, rows);
			png_write_end(png, info);
		} else {
			png_close(&png, file);
			return (OWON_ERROR_PNG);
		}
	} else {
		png_close(&png, file);
		return (OWON_ERROR_PNG);
	}

	png_close(&png, file);
	return (0);
}
