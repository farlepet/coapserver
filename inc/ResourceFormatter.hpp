#ifndef _RESOURCE_FORMATTER_HPP_
#define _RESOURCE_FORMATTER_HPP_

#include <ostream>

#include "Config.hpp"

class ResourceFormatter {
    private:

    public:
        ResourceFormatter();
        virtual ~ResourceFormatter();

        /*virtual int encode(const coap_pdu_t *pdu, std::istream &in) = 0;*/
        virtual int decode(const std::vector<std::byte> &data, std::ostream &out) = 0;

        static ResourceFormatter *createResourceFormatter(const std::string &_resource);
};

#endif /* _RESOURCE_FORMATTER_HPP_ */
