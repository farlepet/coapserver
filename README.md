Coap Server
===========

This CoAP server is designed to be highly configurable to allow it's use with a
variety of IoT devices, for testing or for low-scale use.

__Docker images__:

[![Docker Image Version (latest by date)](https://img.shields.io/docker/v/farlepet/coapserver?label=coapserver)](https://hub.docker.com/r/farlepet/coapserver)

[![Docker Image Version (latest by date)](https://img.shields.io/docker/v/farlepet/coapserver-extra?label=coapserver-extra)](https://hub.docker.com/r/farlepet/coapserver-extra)

__Current Features__:
 - GET, PUT, and POST
    - GET: Returns the value of the resource
    - PUT,POST: Print the sent data, and/or set the current value of the resource
 - Resource observation
 - Multiple data format support
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

__Planned Features__:
 - See GitHub issues: https://github.com/farlepet/coapserver/issues


Running via Docker
------------------

    docker run -it --rm -p 5683:5683/udp \
               --mount type=bind,source=$PWD/config.json,target=/config.json,Z \
               --mount type=bind,source=$PWD/logs/,target=/var/log/coapserver,Z \
               farlepet/coapserver:<version> \
               -c /config.json -l /var/log/coapserver

_OR_

    docker run -it --rm -p 5683:5683/udp \
               --mount type=bind,source=$PWD/config.json,target=/config.json,Z \
               --mount type=bind,source=$PWD/logs/,target=/var/log/coapserver,Z \
               farlepet/coapserver-extra:<version> \
               -c /config.json -l /var/log/coapserver

Building
--------

__Requirements__:
 - `g++`/`clang++`
 - `make`
 - `cmake`
 - libcoap 3: https://github.com/obgm/libcoap
 - nlohmann JSON: https://github.com/nlohmann/json
   - Extra header (`nlohmann/json_fwd.hpp`) required - used to speed up compilation
 - boost >= 1.65.0: https://www.boost.org/

__Available library packages__:
 - Debian: `nlohmann-json3-dev, libcoap3-dev, libboost-all-dev`
 - Arch Linux: `libcoap (AUR), nlohmann-json, boost, msgpack-cxx`

__Building__:

    mkdir build && cd build
    cmake ../ [options]
    make

The following CMake options may be useful:
 - `-DCMAKE_CXX_COMPILER=<...>`
   - Choose which c++ compiler to use
   - Tested with `g++` and `clang++`

Manually building Docker image(s)
------

    # Build base coapserver image
    docker build -t coapserver .
    # Build extra coapserver image - optional (base image must be built first)
    # Adds extra tools including cbor2json, msgpack2json, lz4, gzip, and xz
    docker build -f Dockerfile.extra -t coapserver-extra .

Data Flow
---------

PUT/POST:
 - Request -> Formatter -> Command -> Log

GET:
 - Current Resource Value -> Requestor
