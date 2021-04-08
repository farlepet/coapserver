#ifndef _RESOURCE_FORMATTER_HEX_H
#define _RESOURCE_FORMATTER_HEX_H

#include "ResourceFormatter.hpp"

class ResourceFormatterHex : public ResourceFormatter {
    private:

    public:
        ResourceFormatterHex();

        int decode(const coap_pdu_t *pdu, std::ostream &out);
};

#endif /* _RESOURCE_FORMATTER_HEX_H */
