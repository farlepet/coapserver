#ifndef _RESOURCE_METHOD_HPP_
#define _RESOURCE_METHOD_HPP_

#include <map>
#include <string>
#include <iosfwd>

#include <coap3/coap.h>

class ResourceMethod;

#include "Config.hpp"
#include "ResourceFormatter.hpp"
#include "RequestQueue.hpp"

/* To prevent circular dependency */
class Resource;

class ResourceMethod {
    private:
        const ResourceMethodConfig &config;

        Resource &parentResource;

        RequestQueue &queue;

        std::list<ResourceFormatter *> fmts;

        static std::string methStringify(ResourceMethodType type);

        int executeCmd(std::stringstream &data, std::string cmd);

    public:
        ResourceMethod(Resource &_parentResource, const ResourceMethodConfig &_config, RequestQueue &_queue);

        ~ResourceMethod(void);

        void methodHandler(const coap_pdu_t *request, coap_pdu_t *response, std::vector<std::uint8_t> &data);

        ResourceMethodType getMethodType();

        int handleRequestQueueItem(RequestQueueItem &item);
};

#endif /* _RESOURCE_METHOD_HPP_ */
