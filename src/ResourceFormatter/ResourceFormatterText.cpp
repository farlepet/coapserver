#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterText.hpp"

ResourceFormatterText::ResourceFormatterText() {

}

int ResourceFormatterText::decode(const coap_pdu_t *pdu, std::ostream &out) {
    uint8_t *data;
    size_t   len;
    if(!coap_get_data(pdu, &len, &data)) {
        return -1;
    }
    
    for(size_t i = 0; i < len; i++) {
        out << (char)data[i];
    }

    return 0;
}
