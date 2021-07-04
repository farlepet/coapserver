#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "CoAPServer.hpp"

CoAPServer::CoAPServer(Config &_config) {
    this->endpoint = _config.getEndpoint();

    std::list<ResourceConfig> &resConf = _config.getResources();
    
    std::list<ResourceConfig>::iterator it = resConf.begin();
    for(; it != resConf.end(); it++) {
        this->resources.push_back(new Resource(*it));
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

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
        if ((ainfo->ai_family == AF_INET) ||
            (ainfo->ai_family == AF_INET6)) {
            addr.size = ainfo->ai_addrlen;
            memcpy(&addr.addr.sin6, ainfo->ai_addr, addr.size);
            break;
        }
    }

    freeaddrinfo(res);
    return addr.size;
}