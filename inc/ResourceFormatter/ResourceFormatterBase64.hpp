#ifndef _RESOURCE_FORMATTER_BASE64_H
#define _RESOURCE_FORMATTER_BASE64_H

#include "ResourceFormatter.hpp"

class ResourceFormatterBase64 : public ResourceFormatter {
    private:

    public:
        ResourceFormatterBase64();

        int decode(const std::vector<std::byte> &data, std::ostream &out) override;
};

#endif /* _RESOURCE_FORMATTER_HEX_H */
