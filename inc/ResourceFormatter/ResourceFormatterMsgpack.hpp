#ifndef _RESOURCE_FORMATTER_MSGPACK_H
#define _RESOURCE_FORMATTER_MSGPACK_H

#include "ResourceFormatter.hpp"

class ResourceFormatterMsgpack : public ResourceFormatter {
    private:

    public:
        ResourceFormatterMsgpack();

        int decode(const std::vector<uint8_t> &data, std::ostream &out) override;
};

#endif /* _RESOURCE_FORMATTER_MSGPACK_H */
