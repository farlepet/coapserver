#ifndef _COAP_SERVER_HPP_
#define _COAP_SERVER_HPP_

#include <string>
#include <list>

#include <coap3/coap.h>

#include "Config.hpp"
#include "Resource.hpp"

class CoAPServer {
    private:
        coap_context_t  *ctx = nullptr;
        coap_address_t   addr;
        coap_endpoint_t *endp = nullptr;

        std::list<Resource*> resources;

        EndpointConfig endpoint;

        std::list<ResourceConfig> dynamicResources;

        static int resolveAddress(std::string host, std::string service, coap_address_t &addr);

        template<ResourceMethodType T>
        static void dynamicResourceHandler(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request,
                                           const coap_string_t *query, coap_pdu_t *response);
    public:
        CoAPServer(Config &_config);

        int start();
        
        void handleDynamicRequest(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method);
};

#endif /* _COAP_SERVER_HPP_ */
