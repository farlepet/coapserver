#!/usr/bin/env bats

COAPSERVER=build/coap_server

@test "cmdstdin - enabled" {
    run tests/expect/cmd/cmdstdin_enabled.exp
    [ "$status" -eq 0 ]
}

@test "cmdstdin - disabled" {
    run tests/expect/cmd/cmdstdin_disabled.exp
    [ "$status" -eq 0 ]
}

@test "cmdloginput - enabled" {
    run tests/expect/cmd/cmdloginput_enabled.exp
    [ "$status" -eq 0 ]
}

@test "cmdloginput - disabled" {
    run tests/expect/cmd/cmdloginput_disabled.exp
    [ "$status" -eq 0 ]
}

@test "Environment - COAPSERVER_PAYLOAD" {
    run tests/expect/cmd/env_payload.exp
    [ "$status" -eq 0 ]
}

@test "Environment - COAPSERVER_RESOURCE" {
    run tests/expect/cmd/env_resource.exp
    [ "$status" -eq 0 ]
}

@test "Environment - COAPSERVER_METHOD" {
    run tests/expect/cmd/env_method.exp
    [ "$status" -eq 0 ]
}

@test "Command timeout" {
    run tests/expect/cmd/timeout.exp
    [ "$status" -eq 0 ]
}

