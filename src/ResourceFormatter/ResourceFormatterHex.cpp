#include <iomanip>
#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterHex.hpp"

ResourceFormatterHex::ResourceFormatterHex() {

}

int ResourceFormatterHex::decode(const std::vector<uint8_t> &data, std::ostream &out) {
    out << std::uppercase << std::hex << std::setfill('0') << std::setw(2);
    
    for(size_t i = 0; i < data.size(); i++) {
        out << (int)data[i];
    }

    return 0;
}
