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
 - libcoap: https://github.com/obgm/libcoap
 - nlohmann JSON: https://github.com/nlohmann/json
 - boost: https://www.boost.org/