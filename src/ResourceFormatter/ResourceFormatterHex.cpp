#include <iomanip>
#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterHex.hpp"

ResourceFormatterHex::ResourceFormatterHex() {

}

int ResourceFormatterHex::decode(const std::vector<std::byte> &data, std::ostream &out) {
    out << std::uppercase << std::hex << std::setfill('0');
    
    for(size_t i = 0; i < data.size(); i++) {
        out << std::setw(2) << static_cast<int>(data[i]);
    }

    return 0;
}
