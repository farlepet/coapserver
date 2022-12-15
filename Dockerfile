# syntax=docker/dockerfile:1
FROM --platform=$TARGETPLATFORM alpine:3.15 AS build
RUN apk add --no-cache g++ make cmake gnutls-dev boost-dev nlohmann-json git

COPY . /opt/coapserver

# Build libcoap
RUN set -ex; \
    git clone https://github.com/obgm/libcoap.git -b v4.3.0 /opt/libcoap; \
    cd /opt/libcoap; \
    git apply /opt/coapserver/libcoap_gnutls_session_resumption.patch; \
    mkdir -p /opt/libcoap/build; \
    cd /opt/libcoap/build; \
    cmake .. -DBUILD_SHARED_LIBS=ON -DENABLE_CLIENT_CODE=OFF \
             -DENABLE_EXAMPLES=OFF -DENABLE_DOCS=OFF; \
    cmake --build . --parallel $(nproc) -- install;

# Build coapserver
RUN set -ex; \
    rm -rf /opt/coapserver/build; \
    mkdir -p /opt/coapserver/build; \
    cd /opt/coapserver/build; \
    cmake ..; \
    cmake --build . --parallel $(nproc)

FROM --platform=$TARGETPLATFORM alpine:3.15 AS runtime

COPY --from=build /opt/coapserver/build/coap_server /usr/local/bin/
COPY --from=build /usr/local/lib/libcoap-3.so /usr/local/lib/

RUN apk add --no-cache gnutls boost-program_options libstdc++

ENTRYPOINT ["/usr/local/bin/coap_server"]
EXPOSE 5683
