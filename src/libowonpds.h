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

#ifndef LIBOWONPDS_H_
#define LIBOWONPDS_H_

#include "libowonpds_export.h"

#include <libusb.h>
#include <stdint.h>

/*
 * Owon channel data structure
 *
 * Values are little endian
 *
 * Channel Header
 *		unsigned char name[6];          0
 *		uint32_t fileLength;            6
 *
 * Channel n
 *		unsigned  char channel[3];  0
 *		uint32_t  blockLength;      3
 *		uint32_t  screenLength;     7
 *		uint32_t  sampleLength;     11
 *		uint32_t  slow;             15
 *		uint32_t  timebase;         19
 *		int32_t   offset;           23
 *		uint32_t  sensitivity;      27
 *		uint32_t  attenuation;      31
 *		uint32_t  timeStep?			35
 *		uint32_t  frequency?		39
 *		uint32_t  cycle?			43
 *		float32_t verticalStep?		47
 *		int16_t   data[]            51
 *
 */

// Fixed constants
#define OWON_MAX_CHANNELS 	4

#define OWON_DESC_NAME_LEN 50
#define OWON_SCOPE_NAME_LEN 6
#define OWON_CHANNEL_NAME_LEN 3

#define OWON_BITMAP_WIDTH 640
#define OWON_BITMAP_HEIGHT 480
#define OWON_BITMAP_DEPTH 8
#define OWON_BITMAP_PIXEL_SIZE 3

#ifndef VERSION
#define VERSION "unknown version"
#endif

/**
 * @file
 * @defgroup 	LibOwonPds
 * @{
 * @brief		A userspace driver for Owon PDS oscilloscopes
 * @author		Al Brown
 * @copyright	Copyright &copy; 2015 Al Brown
 *
 */

/**
 * Error codes
 *
 */
typedef enum {
	OWON_ERROR_FORMAT,	/**< Data was in the wrong format */
	OWON_ERROR_PNG, 	/**< Error creating PNG file */
} OWON_ERROR;

/**
 * Type of capture
 */
typedef enum {
	OWON_CHANNEL,	/**< Vector channel */
	OWON_BITMAP		/**< Bitmap */
} OWON_TYPE_T;


/**
 * Channel Data
 */
typedef struct {
	char name[OWON_CHANNEL_NAME_LEN + 1]; 	/**< Name */
	uint32_t samples;						/**< Number of samples */
	double timebase; 						/**< Timebase (s) */
	double slow; 							/**< Most recent time in slow mode (>=100ms timebase) */
	double sample_rate; 					/**< Sample rate */
	double offset; 							/**< Offset (v) */
	double sensitivity; 					/**< Sensitivity (v) */
	unsigned int attenuation; 				/**< Attenuation factor */
	double *vector; 						/**< Level (v) */
} OWON_CHANNEL_T;

/**
 * Scope data
 */
typedef struct {
	unsigned char manufacturer[OWON_DESC_NAME_LEN + 1];	/**< Manufacturer */
	unsigned char product[OWON_DESC_NAME_LEN + 1]; 		/**< Product */
	char name[OWON_SCOPE_NAME_LEN + 1]; 				/**< Name */
	OWON_TYPE_T type; 									/**< Capture type */
	uint32_t file_length; 								/**< File length */

	unsigned channelCount; 								/**< Channels captured */
	OWON_CHANNEL_T channel[OWON_MAX_CHANNELS]; 			/**< Channel data */
	unsigned char *bitmap; 								/**< Bitmap data */

	libusb_context *context; 							/**< libusb context */
	libusb_device_handle *handle; 						/**< libusb handle */
} OWON_SCOPE_T;

LIBOWONPDS_EXPORT char *owon_version();
LIBOWONPDS_EXPORT int owon_open(OWON_SCOPE_T *scope, const int index);
LIBOWONPDS_EXPORT int owon_read(OWON_SCOPE_T *scope);
LIBOWONPDS_EXPORT void owon_free(OWON_SCOPE_T *scope);
LIBOWONPDS_EXPORT void owon_close(OWON_SCOPE_T *scope);

#endif /* LIBOWONPDS_H_ */

/** @}*/
