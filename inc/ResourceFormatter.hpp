#ifndef _RESOURCE_FORMATTER_HPP_
#define _RESOURCE_FORMATTER_HPP_

#include <ostream>

#include <coap2/coap.h>

#include "Config.hpp"

class ResourceFormatter {
    private:

    public:
        ResourceFormatter();

        /*virtual int encode(const coap_pdu_t *pdu, std::istream &in) = 0;*/
        virtual int decode(const std::vector<uint8_t> &data, std::ostream &out) = 0;

        static ResourceFormatter *createResourceFormatter(std::string &_resource);
};

#endif /* _RESOURCE_FORMATTER_HPP_ */
