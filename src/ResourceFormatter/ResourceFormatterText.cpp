#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterText.hpp"

ResourceFormatterText::ResourceFormatterText() {

}

int ResourceFormatterText::decode(const std::vector<std::byte> &data, std::ostream &out) {
    for(size_t i = 0; i < data.size(); i++) {
        out << static_cast<char>(data[i]);
    }

    return 0;
}
