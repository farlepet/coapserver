#include <string>
#include <sstream>
#include <iostream>

#include <stdio.h>

#include <msgpack.hpp>

#include "ResourceFormatter/ResourceFormatterMsgpack.hpp"

ResourceFormatterMsgpack::ResourceFormatterMsgpack() {

}

int ResourceFormatterMsgpack::decode(const std::vector<uint8_t> &data, std::ostream &out) {
    try {
        msgpack::object_handle oh = msgpack::unpack(reinterpret_cast<const char *>(data.data()), data.size());
    
        out << oh.get();
    } catch(msgpack::insufficient_bytes &e) {
        out << "ERROR: msgpack::unpack threw msgpack::insufficient_bytes: " << e.what() << " (" << data.size() << ")";
    }

    return 0;
}
