#ifndef _COAP_SERVER_HPP_
#define _COAP_SERVER_HPP_

#include <string>
#include <list>

#include <coap3/coap.h>

#include "Config.hpp"
#include "Resource.hpp"
#include "RequestQueue.hpp"

class CoAPServer {
    private:
        coap_context_t  *ctx      = nullptr;
        coap_address_t   addr;
        coap_endpoint_t *endp     = nullptr;
        coap_dtls_pki_t *dtls_pki = nullptr;

        RequestQueue    &queue;

        std::list<Resource *> resources;

        EndpointConfig endpoint;

        std::list<ResourceConfig> dynamicResources;

        /** Global log file for non-resource-specific messages. Only to be used by the main thread */
        std::fstream globalLogFile;
        /**
         * @brief libcoap debug message hook
         *
         * @param level Severity
         * @param message Message
         */
        static void logHandler(coap_log_t level, const char *message);

        static int resolveAddress(std::string host, std::string service, coap_address_t &addr);

        template<ResourceMethodType T>
        static void dynamicResourceHandler(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request,
                                           const coap_string_t *query, coap_pdu_t *response);

        int setupSecurity(void);

        static int dtlsValidateCNCallback(const char *cn, const uint8_t *asn1_public_cert, size_t asn1_length,
                                          coap_session_t *session, unsigned int depth, int validated, void *arg);
        static coap_dtls_key_t *dtlsValidateSNICallback(const char *sni, void *arg);
    public:
        CoAPServer(Config &_config, RequestQueue &_queue);

        ~CoAPServer(void);

        /**
         * @brief Initialize CoAP server
         *
         * @return int 0 on sucess, else non-zero
         */
        int init(void);

        /**
         * @brief Listen for incoming connedtions
         *
         * @param timeout_ms How long to wait for packets
         *
         * @return int 0 on success, else non-zero
         */
        int exec(uint32_t timeout_ms);

        void handleDynamicRequest(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method);
};

#endif /* _COAP_SERVER_HPP_ */
