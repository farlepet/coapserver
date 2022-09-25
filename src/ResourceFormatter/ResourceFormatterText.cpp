#include <iomanip>

#include "ResourceFormatter/ResourceFormatterText.hpp"

ResourceFormatterText::ResourceFormatterText() {

}

int ResourceFormatterText::decode(const std::vector<std::byte> &data, std::ostream &out) {
    for(size_t i = 0; i < data.size(); i++) {
        unsigned char ch = static_cast<unsigned char>(data[i]);
        if(std::isprint(ch)) {
            out << ch;
        } else {
            /* Non-printable character */
            std::ios_base::fmtflags flags(out.flags());
            out << "\\x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ch);
            out.flags(flags);
        }
    }

    return 0;
}

