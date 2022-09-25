#ifndef _RESOURCE_FORMATTER_TEXT_H
#define _RESOURCE_FORMATTER_TEXT_H

#include "ResourceFormatter.hpp"

class ResourceFormatterText : public ResourceFormatter {
    private:

    public:
        ResourceFormatterText();

        int decode(const std::vector<std::byte> &data, std::ostream &out) override;
};

#endif /* _RESOURCE_FORMATTER_TEXT_H */
