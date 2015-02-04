#! /usr/bin/env python

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


import copy
from ctypes import *
from ctypes.util import find_library

# Defines from of libowonpds.h
OWON_MAX_CHANNELS = 6
OWON_DESC_NAME_LEN = 50
OWON_SCOPE_NAME_LEN = 6
OWON_CHANNEL_NAME_LEN = 3

## OwonPds

## Wraps the LibOwonPds driver
class OwonPds(object):

    ## Initialise the scope object
    # @param param: index=0    Device index
    # @return Scope object
    def __init__(self, index=0):

        self._index = index
        self._version = owon_version()
        self._scope = Scope()

    ## Get scope data structure
    # @return Scope data structure
    def get_scope(self):
        return self._scope

    ## Get the library version
    # @return Version string
    def get_version(self):
        return self._version

    ## Open communication with the scope
    # @return
    #            - 0 Success
    #            - <0 libusb error
    def open(self):
        return owon_open(byref(self._scope), self._index)

    ## Read from the scope
    # @return
    #            - 0 Success
    #            - <0 libusb error
    def read(self):
        return owon_read(byref(self._scope))

    ## Free allocated channel/bitmap data
    def free(self):
        owon_free(byref(self._scope))

    ## Close scope communication and free memory
    def close(self):
        owon_close(byref(self._scope))

    ## Get a copy of bitmap data
    # @returns Array of successive RGB values
    def get_bitmap(self):
        bitmap = []
        if self._scope.type == 1:
            size = self._scope.bitmapWidth * self._scope.bitmapHeight * self._scope.bitmapChannels
            bitmap = copy.copy(self._scope.bitmap[:size])
        return bitmap

    ## Get a copy of vector data for a channel
    # @param channel Channel to retrieve
    # @returns Array of vectors
    def get_vector(self, channel):

        vector = []
        if self._scope.type == 0 and channel < self._scope.channelCount:
            size = self._scope.channels[channel].samples
            vector = copy.copy(self._scope.channels[channel].vector[:size])
        return vector

    ## Get a copy of vector data for all channels
    # @returns 2D array of vectors
    def get_vectors(self):
        vectors = []
        for channel in range(self._scope.channelCount):
            vectors.append(self.get_vector(channel))

        return vectors

## Channel structure
# (see @ref OWON_CHANNEL_T)
class Channel(Structure):
    _fields_ = [('name', c_char * (OWON_CHANNEL_NAME_LEN + 1)),
                ('samples', c_uint32),
                ('timebase', c_double),
                ('slow', c_double),
                ('sampleRate', c_double),
                ('offset', c_double),
                ('sensitivity', c_double),
                ('attenuation', c_uint),
                ('vector', POINTER(c_double))]


## Scope structure
# (see @ref OWON_SCOPE_T)
class Scope(Structure):
    _fields_ = [('manufacturer', c_char * (OWON_DESC_NAME_LEN + 1)),
                ('product', c_char * (OWON_DESC_NAME_LEN + 1)),
                ('name', c_char * (OWON_SCOPE_NAME_LEN + 1)),
                ('type', c_uint),
                ('fileLength', c_uint32),
                ('channelCount', c_uint),
                ('channels', Channel * (OWON_MAX_CHANNELS + 1)),
                ('bitmapWidth', c_uint),
                ('bitmapHeight', c_uint),
                ('bitmapChannels', c_uint),
                ('bitmap', POINTER(c_char)),
                ('_context', c_void_p),
                ('_handle', c_void_p)]



def libowonpds_load():
    libraries = ['libowonpds.so',
                 'libowonpds.dll',
                 'owonpds.so',
                 'owonpds.dll',
                 find_library('libowonpds'),
                 find_library('owonpds')]

    driver = None
    for library in libraries:
        try:
            driver = cdll.LoadLibrary(library)
            break
        except:
            pass
    else:
        raise ImportError('Could not find LibOwonPds driver')

    return driver

# Load the library
libowonpds = libowonpds_load()

# Library functions
owon_version = libowonpds.owon_version
owon_version.argtypes = []
owon_version.restype = c_char_p

owon_open = libowonpds.owon_open
owon_open.argtypes = [POINTER(Scope), c_uint]
owon_open.restype = c_int

owon_read = libowonpds.owon_read
owon_read.argtypes = [POINTER(Scope)]
owon_read.restype = c_int

owon_free = libowonpds.owon_free
owon_free.argtypes = [POINTER(Scope)]
owon_free.restype = None

owon_close = libowonpds.owon_close
owon_close.argtypes = [POINTER(Scope)]
owon_close.restype = None

# Helper functions
owon_write_csv = libowonpds.owon_write_csv
owon_write_csv.argtypes = [POINTER(Scope), c_char_p, c_bool]
owon_write_csv.restype = None

owon_write_png = libowonpds.owon_write_png
owon_write_png.argtypes = [POINTER(Scope), c_char_p]
owon_write_png.restype = None

if __name__ == '__main__':
    print 'Please run rtlsdr_scan.py'
    exit(1)
