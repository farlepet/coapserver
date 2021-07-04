#ifndef _RESOURCE_METHOD_HPP_
#define _RESOURCE_METHOD_HPP_

#include <map>
#include <string>
#include <iosfwd>

#include <coap2/coap.h>

#include "Config.hpp"
#include "ResourceFormatter.hpp"

/* To prevent circular dependency */
class Resource;

class ResourceMethod {
    private:
        ResourceMethodConfig &config;

        Resource &parentResource;

        ResourceFormatter *fmt = nullptr;

        /* TODO: Only allocate this once. */
        std::map<ResourceMethodType, std::string> methText;

    public:
        ResourceMethod(Resource &_parentResource, ResourceMethodConfig &_config);
        
        void methodHandler(coap_pdu_t *request, coap_pdu_t *response, std::vector<std::uint8_t> &data, std::ostream &log);

        ResourceMethodType getMethodType();
};

#endif /* _RESOURCE_METHOD_HPP_ */
