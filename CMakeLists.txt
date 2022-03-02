cmake_minimum_required(VERSION 3.1)

project(coap_server
        VERSION 0.1
        LANGUAGES CXX)

set(EXECUTABLE_NAME "coap_server")

# C++17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O2")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Setting Clang-specific compiler flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
                         -Weverything \
                         -Wno-unknown-warning-option \
                         -Wno-c++98-compat \
                         -Wno-reserved-identifier \
                         -Wno-reserved-macro-identifier \
                         -Wno-padded")
endif()

# Options
option(DISABLE_MSGPACK "Disable MSGPACK decoding support" OFF)
option(DEBUG "Enable debugging symbols" OFF)

# Required libraries
find_library(COAP_LIBRARY
             NAMES coap-3 coap-3-gnutls coap-3-openssl coap-3-tinydtls coap-3-mbedtls)
find_package(Boost 1.45.0 COMPONENTS program_options)
# Optional libraries
find_package(msgpack)

# Library checks
if(COAP_LIBRARY)
    message(STATUS "Compiling with the following CoAP library: ${COAP_LIBRARY}")
else()
    message(FATAL_ERROR "Could not find a compatible CoAP library!")
endif()

if(!BOOST_FOUND)
    message(FATAL_ERROR "Could not find the required Boost libraries!")
endif()

if(DEBUG)
    message(STATUS "Enabling debug symbols")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# Resource Formatters
add_library(ResourceFormatters
            ${CMAKE_CURRENT_LIST_DIR}/src/ResourceFormatter/ResourceFormatterHex.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/ResourceFormatter/ResourceFormatterText.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/ResourceFormatter/ResourceFormatterBase64.cpp)

if(msgpack_FOUND AND (DISABLE_MSGPACK STREQUAL OFF))
    message(STATUS "Compiling with MSGPACK support")
    set(USE_MSGPACK ON)
    target_sources(ResourceFormatters PRIVATE
                   ${CMAKE_CURRENT_LIST_DIR}/src/ResourceFormatter/ResourceFormatterMsgpack.cpp)
else()
    message(STATUS "Compiling without MSGPACK support")
    set(USE_MSGPACK OFF)
endif()
            

configure_file(${CMAKE_CURRENT_LIST_DIR}/inc/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/inc/config.h)


add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
               ${CMAKE_CURRENT_LIST_DIR}/src/Config.cpp
               ${CMAKE_CURRENT_LIST_DIR}/src/CoAPServer.cpp
               ${CMAKE_CURRENT_LIST_DIR}/src/Resource.cpp
               ${CMAKE_CURRENT_LIST_DIR}/src/ResourceMethod.cpp
               ${CMAKE_CURRENT_LIST_DIR}/src/ResourceFormatter.cpp
               ${CMAKE_CURRENT_LIST_DIR}/src/RequestQueue.cpp)

include_directories(${EXECUTABLE_NAME}
                    ${CMAKE_CURRENT_LIST_DIR}/inc/
                    ${CMAKE_CURRENT_BINARY_DIR}/inc/
                    ${Boost_INCLUDE_DIRS})

target_link_libraries(${EXECUTABLE_NAME}
                      ResourceFormatters
                      ${COAP_LIBRARY}
                      ${Boost_LIBRARIES}
                      stdc++fs)
