#!/usr/bin/env bats

COAPSERVER=build/coap_server

export BATS_TEST_TIMEOUT=5

teardown_file() {
    rm -f data.bin
}

@test "File resource tests" {
    run tests/expect/file_resource.exp
    [ "$status" -eq 0 ]
}

