Coap Server
===========

This CoAP server is designed to be highly configurable to allow it's use with a
variety of IoT devices, for testing or for low-scale use.

Current Features:
 - GET, PUT, and POST
    - GET: Returns the value of the resource
    - PUT,POST: Print the sent data, and/or set the current value of the resource
 - Resource observation
 - Multiple data format support
    - `MSGPACK` - Decode data as MSGPACK
    - `STRING` - Plaintext
    - `HEX` - Raw data represented as hex
 - Logging to a separate file for each resource
 - Templated resource configuration for creating multiple similar resources
 - Multi-block requests
    - Currently no support for multi-block responses

Planned Features:
 - Improved documentation
 - Run commands with request data
 - Set a resource's value from an external program
 - Encoding responses to a GET request
 - Possibly listen to any resource that follows a given format

Requirements:
 - `g++`/`clang++`
 - `make`
 - `cmake`
 - libcoap: https://github.com/obgm/libcoap
 - nlohmann JSON: https://github.com/nlohmann/json
 - boost: https://www.boost.org/

Optional:
 - msgpack-c: https://github.com/msgpack/msgpack-c/tree/cpp_master
   - C++ version required for MSGPACK decoding support

Building
--------
General build process:

    mkdir build && cd build
    cmake ../ [options]
    make

The following CMake options may be useful:
 - `-DDISABLE_MSGPACK=<YES/NO>`
   - Default: NO
   - Choose whether or not to compile in MSGPACK support
 - `-DCMAKE_CXX_COMPILER=<...>`
   - Choose which c++ compiler to use
   - Tested with `g++` and `clang++`

