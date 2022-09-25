#!/usr/bin/env bats

COAPSERVER=build/coap_server

export BATS_TEST_TIMEOUT=5

@test "STRING format" {
    run tests/expect/data_formatting/string.exp
    [ "$status" -eq 0 ]
}

@test "HEX format" {
    run tests/expect/data_formatting/hex.exp
    [ "$status" -eq 0 ]
}

@test "BASE64 format" {
    run tests/expect/data_formatting/base64.exp
    [ "$status" -eq 0 ]
}

