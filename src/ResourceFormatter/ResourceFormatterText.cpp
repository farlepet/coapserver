#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterText.hpp"

ResourceFormatterText::ResourceFormatterText() {

}

int ResourceFormatterText::decode(const std::vector<uint8_t> &data, std::ostream &out) {
    for(size_t i = 0; i < data.size(); i++) {
        out << (char)data[i];
    }

    return 0;
}
