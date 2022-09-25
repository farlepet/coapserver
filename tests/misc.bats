#!/usr/bin/env bats

COAPSERVER=build/coap_server

export BATS_TEST_TIMEOUT=5

@test "SIGINT stops server" {
    run tests/expect/misc/sigint_exit.exp
    [ "$status" -eq 0 ]
}

