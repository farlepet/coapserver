#include <iomanip>
#include <stdio.h>

#include "ResourceFormatter/ResourceFormatterBase64.hpp"

ResourceFormatterBase64::ResourceFormatterBase64() {

}

/* Base64 character set - RFC4648 */
static const char *_base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";

int ResourceFormatterBase64::decode(const std::vector<uint8_t> &data, std::ostream &out) {
    /* 3 bytes / 4 characters at a time */
    for(size_t i = 0; i < data.size(); i += 3) {
        size_t left = data.size() - i;

        uint8_t buff[3];
        buff[0] =               data[i];
        buff[1] = (left >= 2) ? data[i+1] : 0x00;
        buff[2] = (left >= 3) ? data[i+2] : 0x00;

        out << _base64_chars[(buff[0] >> 2)];
        out << _base64_chars[((buff[0] << 4) & 0x30) |
                             (buff[1] >> 4)];

        if(left >= 2) {
            out << _base64_chars[((buff[1] << 2) & 0x3C) |
                                 (buff[2] >> 6)];
        } else {
            out << '=';
        }

        if(left >= 3) {
            out << _base64_chars[buff[2] & 0x3F];
        } else {
            out << '=';
        }
    }

    return 0;
}
