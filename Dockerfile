# syntax=docker/dockerfile:1
FROM alpine:3.15 AS build
RUN apk add --no-cache g++ make cmake gnutls-dev boost-dev nlohmann-json git

# Build libcoap
RUN set -ex; \
    cd /opt; \
    git clone https://github.com/obgm/libcoap.git -b v4.3.0; \
    cd libcoap; \
    mkdir -p build; \
    cd build; \
    cmake .. -DBUILD_SHARED_LIBS=ON; \
    cmake --build . -- install;

# Build coapserver
COPY . /opt/coapserver
RUN set -ex; \
    rm -rf /opt/coapserver/build; \
    mkdir -p /opt/coapserver/build; \
    cd /opt/coapserver/build; \
    cmake ..; \
    make

FROM alpine:3.15 AS runtime

COPY --from=build /opt/coapserver/build/coap_server /usr/local/bin/
COPY --from=build /usr/local/lib/libcoap-3.so /usr/local/lib/

RUN apk add --no-cache gnutls boost

ENTRYPOINT ["/usr/local/bin/coap_server"]
EXPOSE 5683
