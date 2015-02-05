# LibOwonPds #

Copyright 2015 Al Brown

al [at] eartoearoak.com


A userspace driver for Owon PDS oscilloscopes.

Tested with a PDS5022S, but should work with variants.

More details can be found [here](http://eartoearoak.com/software/libowonpds).

Tested on:

- Windows 7 (x86_64)
- Windows XP (x86)
- Ubuntu 14.04 (x86_64)

## Building ##

**Requirements**

- [CMake 2.8](http://www.cmake.org/)
- [libusb-1.0](http://www.libusb.org/)
- [libpng](http://www.libpng.org/pub/png/libpng.html)
- [Python](https://www.python.org/) (Optional)
- [wxPython](http://www.wxpython.org/) (Optional)

**Build & Install**

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Usage ##

**Utility**

`owonpds [filename]`

Print information about the scope data and optionally save it to a CSV or PNG file depending on the scope mode.

**Python**

An Python test script 'owon_scope.py' is included in the 'src/' directory to display vector data from the scope

**C Library**

```
void main(void){
    OWON_SCOPE_T scope;
    owon_open(&scope, 0); // Success if zero
    owon_read(&scope);    // Success if zero
	/* scope structure now holds captured data
	 * do something with it and maybe owon_read() again...
     */
    owon_close(&scope);
}
```

**Python Wrapper**

```
import libowonscope

def main():
	scopeObj = libowonpds.OwonPds()
	scopeObj.open() # Success if zero
	scopeObj.read() # Success if zero
	data = scopeObj.get_scope()
	# data holds the captured data
	# owon_read() again or
	scopeObj.close()
```

## Documentation ##

- [General information](http://eartoearoak.com/software/libowonpds)
- [API](http://eartoearoak.com/software/libowonpds/libowonpds-documentation)


## Known Limitations ##
- Streaming data is not available, the incoming data is not time-stamped.
- Polling faster than 7Hz causes the oscilloscope to reboot after a while (PDS5022S - W5022S08530496 v4.1) 

## License ##

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
