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

#include "libowonpds.h"

#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define USB_VID 0x5345
#define USB_PID 0x1234

#define USB_CONFIG 1
#define USB_INTERFACE 0

#define WRITE_ENDPOINT	0x03
#define READ_ENDPOINT  0x81

#define TIMEOUT 2000

#define CMD_START "START"

#define HEADER_SIZE 12
#define HEADER_TYPE 8

#define FILE_HEADER_SIZE 10
#define CHANNEL_HEADER_SIZE 51
#define BITMAP_HEADER_SIZE 54

#define FILE_SIZE 0
#define FILE_TYPE 8

#define SCOPE_NAME 0
#define SCOPE_TYPE 3

#define CH_NAME 0
#define CH_BLOCK_LEN 3
#define CH_SAMPLE_LEN 11
#define CH_SLOW 15
#define CH_TIMEBASE 19
#define CH_OFFSET 23
#define CH_SENS 27
#define CH_ATTEN 31

#define ID_VECTOR "SPB"
#define ID_BITMAP "BM"

#define SCALE_T 10
#define SCALE_V 25

static double TIMEBASE[32] = { 0.000005, 0.00001, 0.000025, 0.00005, 0.0001,
		0.00025, 0.0005, 0.001, 0.0025, 0.005, 0.01, 0.025, 0.05, 0.1, 0.25,
		0.5, 1, 2.5, 5, 10, 25, 50, 100, 250, 500, 1000, 2500, 5000, 10000,
		25000, 50000, 100000 };

static double SENSITIVITY[21] = { 0.002, 0.005, 0.01, 0.02, 0.05, 0.1, 0.2, 0.5,
		1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000 };

void error(const char *message) {

	fprintf(stderr, message);
	fprintf(stderr, "\n");
}

// Convert from little endian
uint32_t data_to_uint(const unsigned char* from, const uint length) {

	uint32_t convert = 0;
	memcpy(&convert, from, length);

	return (le32toh(convert));
}

// Convert from little endian
int32_t data_to_int(const unsigned char* from, const uint length) {

	int32_t convert = 0;
	memcpy(&convert, from, length);

	return (le32toh(convert));
}

// Exponent of 10
uint32_t power10(const uint32_t exponent){

	uint32_t i;
	uint32_t power = 1;

	for (i = 0; i < exponent; ++i)
		power *= 10;

	return(power);
}

// Scale little endian vector data to volts
void scale_vector(OWON_CHANNEL_T *channel, const unsigned char *data) {

	uint32_t length = channel->samples;
	channel->vector = malloc(sizeof(double) * length);

	if (channel->vector) {
		uint32_t i;
		for (i = 0; i < length; i++) {
			// Endian conversion
			short value = (short) (data[i * 2] | (data[i * 2 + 1]) << 8);
			value = le16toh(value);
			channel->vector[i] = value * channel->sensitivity / SCALE_V;
		}

	} else
		error("Failed to allocate vector memory");
}

// Decode channel data
void decode_channel(OWON_SCOPE_T *scope, unsigned char *data) {

	memcpy(scope->name, &data[SCOPE_NAME], OWON_SCOPE_NAME_LEN);

	// Check if data is matching vector type
	if (data[SCOPE_TYPE] == 'V' || data[SCOPE_TYPE] == 'W'
			|| data[SCOPE_TYPE] == 'X') {
		scope->type = OWON_CHANNEL;

		unsigned char *current = data + FILE_HEADER_SIZE;
		unsigned channel_num = 0;

		// Loop over each channel block extracting data
		OWON_CHANNEL_T *channel;
		while (current - data < scope->file_length) {
			uint32_t blockSize;
			channel = &scope->channel[channel_num];

			memcpy(channel->name, &current[CH_NAME], OWON_CHANNEL_NAME_LEN);

			blockSize = data_to_uint(&current[CH_BLOCK_LEN], 4);
			channel->samples = data_to_uint(&current[CH_SAMPLE_LEN], 4);

			uint32_t timebase_index = data_to_uint(&current[CH_TIMEBASE], 1);
			channel->timebase = TIMEBASE[timebase_index] / 1000;
			channel->sample_rate = channel->samples / channel->timebase
					/ SCALE_T;

			uint32_t slow = data_to_uint(&current[CH_SLOW], 4);
			channel->slow = slow / channel->sample_rate;

			uint32_t attenuation_index = data_to_uint(&current[CH_ATTEN], 4);
			channel->attenuation = power10(attenuation_index);

			uint32_t sensitivity_index = data_to_uint(&current[CH_SENS], 1);
			double sensitivity = SENSITIVITY[sensitivity_index];
			if (data[3] == 'W' || data[3] == 'X')
				sensitivity -= 0.0000005;
			channel->sensitivity = sensitivity * channel->attenuation;

			int32_t offset_index = data_to_int(&current[CH_OFFSET], 4);
			channel->offset = offset_index * channel->sensitivity / SCALE_V;

			scale_vector(channel, current + CHANNEL_HEADER_SIZE);

			current += blockSize + OWON_CHANNEL_NAME_LEN;

			if (channel_num < OWON_MAX_CHANNELS)
				channel_num++;
			else
				break;
		}
		scope->channelCount = channel_num;
	} else
		error("Unknown vector type");
}

// Decode bitmap data
void decode_bitmap(OWON_SCOPE_T *scope, unsigned char *data) {

	scope->type = OWON_BITMAP;
	scope->bitmap = malloc(scope->file_length);
	unsigned char *image = data + BITMAP_HEADER_SIZE;
	if (scope->bitmap) {
		unsigned i;
		unsigned pixel_size = sizeof(char) * OWON_BITMAP_PIXEL_SIZE;
		unsigned row_size = OWON_BITMAP_WIDTH * pixel_size;
		// Vertical flip
		for (i = 0; i < OWON_BITMAP_HEIGHT; i++) {
			memcpy(&scope->bitmap[i * row_size],
					&image[(OWON_BITMAP_HEIGHT - i) * row_size], row_size);
		}
	} else
		error("Failed to allocate bitmap memory");
}

// Decode a file based on it's id
bool decode_file(OWON_SCOPE_T *scope, unsigned char *data) {

	if (strncmp(ID_VECTOR, (char *) data, sizeof(ID_VECTOR) - 1) == 0)
		decode_channel(scope, data);
	else if (strncmp(ID_BITMAP, (char *) data, sizeof(ID_BITMAP) - 1) == 0) {
		decode_bitmap(scope, data);
	} else
		return (false);

	return (true);
}

// Open an Owon USB PDS device
int open_device(OWON_SCOPE_T *scope, const int index) {

	libusb_device **devices;
	libusb_device *device = NULL;
	struct libusb_device_descriptor descriptor;
	int error_code = -1;
	ssize_t total;
	int i;
	int count = 0;

	// Get devices
	total = libusb_get_device_list(scope->context, &devices);
	for (i = 0; i < total; i++) {
		libusb_get_device_descriptor(devices[i], &descriptor);

		// Filter by id
		if (descriptor.idVendor == USB_VID && descriptor.idProduct == USB_PID)
			count++;

		if (index == count - 1) {
			device = devices[i];
			break;
		}
	}

	if (device) {
		// Open the device
		error_code = libusb_open(device, &scope->handle);
		if (error_code == LIBUSB_SUCCESS)
			error_code = libusb_set_configuration(scope->handle, USB_CONFIG);
		if (error_code == LIBUSB_SUCCESS)
			error_code = libusb_claim_interface(scope->handle,
			USB_INTERFACE);
		if (error_code == LIBUSB_SUCCESS) {
			libusb_get_string_descriptor_ascii(scope->handle, 1,
					scope->manufacturer, OWON_DESC_NAME_LEN);
			libusb_get_string_descriptor_ascii(scope->handle, 2, scope->product,
			OWON_DESC_NAME_LEN);
		}
	} else
		error_code = LIBUSB_ERROR_NO_DEVICE;

	return (error_code);
}

/**
 * Open communications with the scope
 *
 * Must be called before reading the scope
 *
 * @param scope		Scope struct to be initialised
 * @param index		Device index
 * @return 0 Success,  <0 libusb error
 *
 */
int owon_open(OWON_SCOPE_T *scope, const int index) {

	int errorCode;

	memset(scope, 0, sizeof(OWON_SCOPE_T));

	errorCode = libusb_init(&scope->context);

	if (errorCode == LIBUSB_SUCCESS)
		errorCode = open_device(scope, index);

	return (errorCode);
}

/**
 * Capture data from the device
 *
 * @param scope 	Initialised scope struct to read data to
 * @return 0 Success,  <0 libusb error
 *
 */
int owon_read(OWON_SCOPE_T *scope) {

	int errorCode = LIBUSB_SUCCESS;
	int transferred = 0;

	owon_free(scope);

	if (scope->handle) {
		// Send start command
		errorCode = libusb_bulk_transfer(scope->handle, WRITE_ENDPOINT,
				(unsigned char *) CMD_START, sizeof(CMD_START), &transferred,
				TIMEOUT);
		if (errorCode == LIBUSB_SUCCESS) {
			// Get header
			unsigned char header[HEADER_SIZE];
			errorCode = libusb_bulk_transfer(scope->handle, READ_ENDPOINT,
					header, sizeof(header), &transferred, TIMEOUT);

			uint32_t fileLength = data_to_uint(&header[FILE_SIZE], 3);
			if (header[FILE_TYPE] == 1)
				fileLength += BITMAP_HEADER_SIZE;

			// Get data
			unsigned char *data = malloc(fileLength);
			if (data) {
				errorCode = libusb_bulk_transfer(scope->handle,
				READ_ENDPOINT, data, (int)fileLength, &transferred,
				TIMEOUT);
				if (errorCode == LIBUSB_SUCCESS) {
					scope->file_length = fileLength;
					if (!decode_file(scope, data)) {
						errorCode = OWON_ERROR_FORMAT;
						error("Unknown format");
					}
				}
				free(data);
			} else
				error("Failed to allocate transfer memory");
		}
	} else
		errorCode = LIBUSB_ERROR_NO_DEVICE;

	return (errorCode);
}

/**
 * Free capture data
 *
 * @param scope Scope struct to free captured data memory from
 *
 */
void owon_free(OWON_SCOPE_T *scope) {

	if (scope) {
		unsigned i;
		for (i = 0; i < scope->channelCount; i++) {
			OWON_CHANNEL_T *channel;
			channel = &scope->channel[i];
			if (channel->vector) {
				free(channel->vector);
				channel->vector = NULL;
			}
		}
		scope->channelCount = 0;
		if (scope->bitmap)
			free(scope->bitmap);
	}
}

/**
 * Close communications with the scope
 *
 * @param scope
 *
 */
void owon_close(OWON_SCOPE_T *scope) {

	if (scope) {
		owon_free(scope);
		if (scope->handle) {
			libusb_release_interface(scope->handle, USB_INTERFACE);
			libusb_close(scope->handle);
		}
		if (scope->context)
			libusb_exit(scope->context);
	}
}
