#!/usr/bin/env bats

COAPSERVER=build/coap_server

export BATS_TEST_TIMEOUT=5

setup_file() {
    # Certificate authority
    openssl genrsa -out coapserver-ca.key 4096
    openssl req -x509 -new -nodes -key coapserver-ca.key -sha256 -days 365 -out coapserver-ca.cert.pem -subj "/C=/"

    # Server key
    openssl genrsa -out coapserver.key 4096
    openssl req -new -key coapserver.key -out coapserver.csr -subj "/C=/"
    openssl x509 -req -days 1 -in coapserver.csr -CA coapserver-ca.cert.pem -CAkey coapserver-ca.key -CAcreateserial -out coapserver.cert.pem

    # Client key
    openssl genrsa -out coap-client.key 4096
    openssl req -new -key coap-client.key -out coap-client.csr -subj "/C=/"
    openssl x509 -req -days 365 -in coap-client.csr -CA coapserver-ca.cert.pem -CAkey coapserver-ca.key -CAcreateserial -out coap-client.cert.pem
}

teardown_file() {
    # Delete certificate files
    rm -f coapserver-ca.key coapserver-ca.cert.pem coapserver-ca.cert.srl
    rm -f coapserver.key coapserver.csr coapserver.cert.pem
    rm -f coap-client.key coap-client.csr coap-client.cert.pem
}

@test "CoAP over UDP" {
    run tests/expect/protocols/protocol_udp.exp
    [ "$status" -eq 0 ]
}

@test "CoAP over DTLS" {
    run tests/expect/protocols/protocol_dtls.exp
    [ "$status" -eq 0 ]
}

@test "CoAP over TCP" {
    run tests/expect/protocols/protocol_tcp.exp
    [ "$status" -eq 0 ]
}

@test "CoAP over TLS" {
    run tests/expect/protocols/protocol_tls.exp
    [ "$status" -eq 0 ]
}

