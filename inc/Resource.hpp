#ifndef _RESOURCE_HPP_
#define _RESOURCE_HPP_

#include <fstream>
#include <string>
#include <list>

#include <coap3/coap.h>

#include "Config.hpp"
#include "ResourceMethod.hpp"

class Resource {
    private:
        ResourceConfig &config;

        std::list<ResourceMethod> methods;
        
        std::string value;

        coap_resource_t *res;

        std::fstream logFile;

        template<ResourceMethodType T>
        static void coapHandlerX(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request,
                                 const coap_string_t *query, coap_pdu_t *response);

        int sessionGetData(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, std::vector<uint8_t> &dataOut);

    public:
        Resource(ResourceConfig &_config);

        int attach(coap_context_t *ctx);

        void handleCoAPRequest(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method);

        void        setValue(std::string _value);
        std::string getValue();

        std::string getPath();

        unsigned    getMaxAge();
};

#endif /* _RESOURCE_HPP_ */
