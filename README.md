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
    - `STRING`  - Plaintext
    - `HEX`     - Raw data represented as hex
    - `BASE64`  - Raw data represented as base64
 - Logging to a separate file for each resource
 - Execute external command on request data
   - By piping request data via stdin, or passing in via environment variable
 - Templated resource configuration for creating multiple similar resources
 - Regex-based on-the-fly resource creation
 - Multi-block requests
    - Currently no support for multi-block responses

Planned Features:
 - See GitHub issues: https://github.com/farlepet/coapserver/issues

Requirements:
 - `g++`/`clang++`
 - `make`
 - `cmake`
 - libcoap 3: https://github.com/obgm/libcoap
 - nlohmann JSON: https://github.com/nlohmann/json
   - Extra header (`nlohmann/json_fwd.hpp`) required - used to speed up compilation
 - boost >= 1.65.0: https://www.boost.org/

Optional:
 - msgpack-c: https://github.com/msgpack/msgpack-c/tree/cpp_master
   - C++ version required for MSGPACK decoding support

Available library packages:
 - Debian: `nlohmann-json3-dev, libcoap3-dev, libboost-all-dev`
 - Arch Linux: `libcoap (AUR), nlohmann-json, boost, msgpack-cxx`


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

Data Flow
---------

PUT/POST:
 - Request -> Formatter -> Command -> Log

GET:
 - Command -> Requestor
