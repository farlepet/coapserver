#!/usr/bin/env bats

COAPSERVER=build/coap_server

export BATS_TEST_TIMEOUT=5

@test "File resource tests" {
    run tests/expect/file_resource.exp
    [ "$status" -eq 0 ]
    rm -f data.bin
}

