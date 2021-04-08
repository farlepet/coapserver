#include <iomanip>
#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterHex.hpp"

ResourceFormatterHex::ResourceFormatterHex() {

}

int ResourceFormatterHex::decode(const coap_pdu_t *pdu, std::ostream &out) {
    uint8_t *data;
    size_t   len;
    if(!coap_get_data(pdu, &len, &data)) {
        return -1;
    }
    
    out << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    for(size_t i = 0; i < len; i++) {
        if(!(i % 40)) {
            out << std::endl << "    ";
        }
        out << (int)data[i];
    }

    return 0;
}
