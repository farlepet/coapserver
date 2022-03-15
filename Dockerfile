# syntax=docker/dockerfile:1
FROM alpine:3.15 AS build
RUN apk add --no-cache g++ make cmake libcoap-dev boost-dev nlohmann-json

COPY . /opt/coapserver
RUN set -ex;                         \
    rm -r /opt/coapserver/build;     \
    mkdir -p /opt/coapserver/build;  \
    cd /opt/coapserver/build;        \
    cmake .. -DDISABLE_MSGPACK=true; \
    make


FROM alpine:3.15 AS runtime

COPY --from=build /opt/coapserver/build/coap_server /usr/local/bin/

RUN apk add --no-cache libcoap boost

ENTRYPOINT ["/usr/local/bin/coap_server"]
EXPOSE 5683
