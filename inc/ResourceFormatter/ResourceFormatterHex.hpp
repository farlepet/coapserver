#ifndef _RESOURCE_FORMATTER_HEX_H
#define _RESOURCE_FORMATTER_HEX_H

#include "ResourceFormatter.hpp"

class ResourceFormatterHex : public ResourceFormatter {
    private:

    public:
        ResourceFormatterHex();

        int decode(const std::vector<uint8_t> &data, std::ostream &out) override;
};

#endif /* _RESOURCE_FORMATTER_HEX_H */
