#ifndef _RESOURCE_FORMATTER_MSGPACK_H
#define _RESOURCE_FORMATTER_MSGPACK_H

#include "ResourceFormatter.hpp"

class ResourceFormatterMsgpack : public ResourceFormatter {
    private:

    public:
        ResourceFormatterMsgpack();

        int decode(const coap_pdu_t *pdu, std::ostream &out);
};

#endif /* _RESOURCE_FORMATTER_MSGPACK_H */
