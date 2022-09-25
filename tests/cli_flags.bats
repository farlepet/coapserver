#!/usr/bin/env bats

COAPSERVER=build/coap_server

export BATS_TEST_TIMEOUT=5

# @todo Consolidate return values

@test "Error on non-existant config" {
    run $COAPSERVER -c badconfig.json
    [ "$status" -eq 134 ]
    [ "${lines[1]}" = "  what():  Could not open config file 'badconfig.json' for reading!" ]
}

@test "Error on port out of range" {
    run $COAPSERVER -p 0
    [ "$status" -eq 1 ]
    [ "${lines[0]}" = "Port must be an integer between 1 and 65536 (inclusive)" ]
    run $COAPSERVER -p 65537
    [ "$status" -eq 1 ]
    [ "${lines[0]}" = "Port must be an integer between 1 and 65536 (inclusive)" ]
    run $COAPSERVER -p -1
    [ "$status" -eq 1 ]
    [ "${lines[0]}" = "Port must be an integer between 1 and 65536 (inclusive)" ]
}

#@test "Error on malformed IP address" {
#    run $COAPSERVER -a 0
#    [ "$status" -eq 255 ]
#    [ "${lines[2]}" = "ERROR: Could not add CoAP endpoint." ]
#}

