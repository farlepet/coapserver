#ifndef _RESOURCE_FORMATTER_TEXT_H
#define _RESOURCE_FORMATTER_TEXT_H

#include "ResourceFormatter.hpp"

class ResourceFormatterText : public ResourceFormatter {
    private:

    public:
        ResourceFormatterText();

        int decode(const coap_pdu_t *pdu, std::ostream &out);
};

#endif /* _RESOURCE_FORMATTER_TEXT_H */
