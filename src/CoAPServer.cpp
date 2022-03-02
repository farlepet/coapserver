#include <iostream>
#include <regex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "CoAPServer.hpp"

CoAPServer::CoAPServer(Config &_config, RequestQueue &_queue) :
queue(_queue) {
    this->endpoint = _config.getEndpoint();

    std::list<ResourceConfig> &resConf = _config.getResources();
    
    std::list<ResourceConfig>::iterator it = resConf.begin();
    for(; it != resConf.end(); it++) {
        if(it->dynamic) {
            this->dynamicResources.push_back(*it);
        } else {
            this->resources.push_back(new Resource(*it, this->queue));
        }
    }
}

int CoAPServer::start() {
    int ret = 0;
    
    std::cerr << "Binding to " << this->endpoint.address << ":"
              << this->endpoint.port << std::endl;
    
    if(!CoAPServer::resolveAddress(this->endpoint.address, this->endpoint.port, this->addr)) {
        std::cerr << "ERROR: Failed to resolve address." << std::endl;
        ret = 1;
    } else if(!(this->ctx = coap_new_context(nullptr))) {
        std::cerr << "ERROR: Could not create CoAP context." << std::endl;
        ret = 1;
    } else if(!(this->endp = coap_new_endpoint(this->ctx, &this->addr, COAP_PROTO_UDP))) {
        std::cerr << "ERROR: Could not add CoAP endpoint." << std::endl;
        ret = 1;
    }
    
    if(!ret) {
        std::list<Resource *>::iterator it = this->resources.begin();
        for(; it != this->resources.end(); it++) {
            if((*it)->attach(this->ctx)) {
                ret = 1;
                break;
            }
        }
    }

    if(!ret && this->dynamicResources.size()) {
        coap_resource_t *res = coap_resource_unknown_init(CoAPServer::dynamicResourceHandler<ResourceMethodType::PUT>);
        coap_register_handler(res, COAP_REQUEST_POST, CoAPServer::dynamicResourceHandler<ResourceMethodType::POST>);
        coap_register_handler(res, COAP_REQUEST_GET, CoAPServer::dynamicResourceHandler<ResourceMethodType::GET>);
        coap_resource_set_userdata(res, this);
        coap_add_resource(this->ctx, res);
    }

    if(!ret) {
        coap_context_set_block_mode(this->ctx, COAP_BLOCK_USE_LIBCOAP);
        const uint16_t cache_ignore_options[] = { COAP_OPTION_BLOCK1,
                                                  COAP_OPTION_BLOCK1,
                                                  COAP_OPTION_MAXAGE,
                                                  COAP_OPTION_IF_NONE_MATCH };
        coap_cache_ignore_options(this->ctx, cache_ignore_options, sizeof(cache_ignore_options) / sizeof(cache_ignore_options[0]));
    }

    while(!ret) {
        /* TODO: Spawn thread */
        coap_io_process(this->ctx, COAP_IO_WAIT);
    }

    coap_free_context(ctx);
    coap_cleanup();
    
    return ret;
}

int CoAPServer::resolveAddress(std::string host, std::string service, coap_address_t &addr) {
    struct addrinfo *res, *ainfo;
    struct addrinfo hints;
    int             error;

    addr.size = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family   = AF_UNSPEC;

    error = getaddrinfo(host.c_str(), service.c_str(), &hints, &res);

    if (error != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(error) << std::endl;
        return error;
    }

    for (ainfo = res; ainfo != nullptr; ainfo = ainfo->ai_next) {
        if ((ainfo->ai_family == AF_INET) ||
            (ainfo->ai_family == AF_INET6)) {
            addr.size = ainfo->ai_addrlen;
            memcpy(&addr.addr.sin6, ainfo->ai_addr, addr.size);
            break;
        }
    }

    freeaddrinfo(res);
    return static_cast<int>(addr.size);
}

static std::string _regexReplace(std::string in, std::smatch sm) {
    std::ostringstream out;
    for(size_t i = 0; i < in.size(); i++) {
        if(in[i] == '$') {
            i++;
            if(in[i] == '$') {
                out << in[i];
            } else if(isdigit(in[i])) {
                size_t idx = 0;
                while(isdigit(in[i])) {
                    idx = (idx * 10) + static_cast<size_t>(in[i] - '0');
                    i++;
                }
                i--;
                
                if(idx > sm.size()) {
                    std::cerr << "ERROR: Insufficient regex matches for template string: " << in << std::endl;
                    return "";
                } else {
                    out << sm[idx];
                }
            } else {
                std::cerr << "ERROR: Improperly formatted template string: `" + in + "`!" << std::endl;
                return "";
            }
        } else {
            out << in[i];
        }
    }
    return out.str();
}

void CoAPServer::handleDynamicRequest(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method) {
    coap_string_t *_uri = coap_get_uri_path(request);
    if(_uri == nullptr) {
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_NOT_FOUND);
        return;
    }
    std::string uri(reinterpret_cast<const char *>(_uri->s));
    coap_delete_string(_uri);

    std::list<ResourceConfig>::iterator it = this->dynamicResources.begin();
    for(; it != this->dynamicResources.end(); it++) {

        std::regex  ex(it->resourcePath);
        std::smatch sm;
        if(std::regex_match(uri, sm, ex)) {
            /* Check if this resource contains the requested method */
            std::list<ResourceMethodConfig>::iterator mi = it->methods.begin();
            for(; mi != it->methods.end(); mi++) {
                if(mi->type == method) { break; }
            }
            if(mi == it->methods.end()) {
                std::cerr << "ERROR: Bad method on dynamic resource " << it->resourcePath << std::endl;
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_NOT_ALLOWED);
                return;
            }

            ResourceConfig *cfg = new ResourceConfig(*it);
            cfg->resourcePath   = uri;
            cfg->resourceName   = _regexReplace(cfg->resourceName, sm);
            cfg->logFile        = _regexReplace(cfg->logFile, sm);

            Resource *res = new Resource(*cfg, this->queue);
            if(res == nullptr) {
                std::cerr << "ERROR: Could not create Resource object" << std::endl;
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                return;
            }

            if(res->attach(this->ctx)) {
                std::cerr << "ERROR: Could not attach Resource object" << std::endl;
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                return;
            }
            this->resources.push_back(res);
            /* We will need to manually forward the first request */
            res->handleCoAPRequest(request, response, session, method);
            return;
        }
    }

    std::cerr << "Unhandled Dynamic URI: " << uri << std::endl;
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_NOT_FOUND);
}

template<ResourceMethodType T>
void CoAPServer::dynamicResourceHandler(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request,
                                        const coap_string_t *query, coap_pdu_t *response) {
    (void)query;
    
    CoAPServer *srv = static_cast<CoAPServer *>(coap_resource_get_userdata(resource));

    srv->handleDynamicRequest(request, response, session, T);
}
