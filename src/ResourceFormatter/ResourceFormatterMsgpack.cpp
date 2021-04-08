#include <string>
#include <sstream>
#include <iostream>

#include <stdio.h>

#include <msgpack.hpp>

#include "ResourceFormatter/ResourceFormatterMsgpack.hpp"

ResourceFormatterMsgpack::ResourceFormatterMsgpack() {

}

int ResourceFormatterMsgpack::decode(const coap_pdu_t *pdu, std::ostream &out) {
    uint8_t *data;
    size_t   len;
    if(!coap_get_data(pdu, &len, &data)) {
        return -1;
    }
    
    msgpack::object_handle oh = msgpack::unpack((const char *)data, len);

    out << oh.get();

    return 0;
}
