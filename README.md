Coap Server
===========

This CoAP server is designed to be highly configurable to allow its use with a
variety of IoT devices, for testing or for low-scale use.

__Docker images__:

[![Docker Image Version (latest by date)](https://img.shields.io/docker/v/farlepet/coapserver?label=coapserver)](https://hub.docker.com/r/farlepet/coapserver)

[![Docker Image Version (latest by date)](https://img.shields.io/docker/v/farlepet/coapserver-extra?label=coapserver-extra)](https://hub.docker.com/r/farlepet/coapserver-extra)

__Current Features__:
 - CoAP via UDP, DTLS, TCP, and TLS
    - Currently only PEM keys/certificates are supported
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

`coapserver` images are build to support amd64, arm64, armv6, armv6, and i386
targets. `coapserver-extra` is currently only built to support amd64, due to
`-Wnarrowing` errors thrown by the `msgpack-tooks` build on other platforms.

NOTE: Depending on the host system (for instance, podman on CentOS), it may be
required to append a `,Z` to the end of both bind mounts to work with SELinux.

    docker run -it --rm -p 5683:5683/udp \
               --mount type=bind,source=$PWD/config.json,target=/config.json \
               --mount type=bind,source=$PWD/logs/,target=/var/log/coapserver \
               farlepet/coapserver:<version> \
               -c /config.json -l /var/log/coapserver

_OR_

    docker run -it --rm -p 5683:5683/udp \
               --mount type=bind,source=$PWD/config.json,target=/config.json \
               --mount type=bind,source=$PWD/logs/,target=/var/log/coapserver \
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

NOTE: Dockerfiles now expect to be run using `docker buildx build`, and will not
work with standard `docker build` (unless having previously run `docker buildx install`).

    # Build base coapserver image
    docker buildx build -t coapserver .
    # Build extra coapserver image - optional (base image must be built first)
    # Adds extra tools including cbor2json, msgpack2json, lz4, gzip, and xz
    docker buildx build -f Dockerfile.extra -t coapserver-extra .


Running Test Cases
------------------

Test cases are written in [BATS](https://github.com/bats-core/bats-core) and
[expect](https://core.tcl-lang.org/expect/index), both of these must be installed
prior to running a test case.

To run the entire test suite:

    bats tests/

To run a specific set of tests:

    bats tests/<test>.bat

This must be run from the root of the repository, and the `coap_server` binary
must be present at `build/coap_server`, as with the build process mentioned
above.


Data Flow
---------

PUT/POST:
 - Request -> Formatter -> Command -> Log

GET:
 - Current Resource Value -> Requestor

