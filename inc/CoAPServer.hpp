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

        static int resolveAddress(std::string host, std::string service, coap_address_t &addr);
    public:
        CoAPServer(Config &_config);

        int start();
};

#endif /* _COAP_SERVER_HPP_ */
