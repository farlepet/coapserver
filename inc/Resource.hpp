#ifndef _RESOURCE_HPP_
#define _RESOURCE_HPP_

#include <fstream>
#include <vector>
#include <string>
#include <list>

#include <coap3/coap.h>

#include "Config.hpp"
#include "ResourceMethod.hpp"
#include "RequestQueue.hpp"

class Resource {
    private:
        const ResourceConfig *config;

        RequestQueue   &queue;

        std::list<ResourceMethod *> methods;

        //std::string value;
        /** Current value of the resource */
        std::vector<std::byte> value;

        coap_resource_t *res;

        std::fstream logFile;

        template<ResourceMethodType T>
        static void coapHandlerX(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request,
                                 const coap_string_t *query, coap_pdu_t *response);

        int sessionGetData(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, std::vector<std::byte> &dataOut);

    public:
        Resource(const ResourceConfig *_config, RequestQueue &_queue, std::string &_logDir);

        ~Resource(void);

        int attach(coap_context_t *ctx);

        void handleCoAPRequest(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method);

        void                   setValue(const std::string &_value);
        void                   setValue(const std::vector<std::byte> &_value);
        std::vector<std::byte> getValue();

        std::string getPath();

        unsigned    getMaxAge();

        void        writeLog(const std::string &str);
};

#endif /* _RESOURCE_HPP_ */
