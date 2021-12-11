#include <iostream>
#include <filesystem>

#include "Resource.hpp"

#include "coap2/resource.h"

Resource::Resource(ResourceConfig &_config) :
config(_config) {
    std::list<ResourceMethodConfig> &methodConfigs = this->config.methods;

    std::list<ResourceMethodConfig>::iterator it = methodConfigs.begin();
    for(; it != methodConfigs.end(); it++) {
        this->methods.push_back(ResourceMethod(*this, *it));
        if(it->logValue && !this->logFile.is_open()) {
            if(this->config.logFile.size()) {
                if(this->config.logFile.find(std::filesystem::path::preferred_separator) != std::string::npos) {
                    /* File is nested in a directory, make sure it exists. */
                    std::filesystem::create_directories(std::filesystem::path(this->config.logFile).parent_path());
                }
                this->logFile.open(this->config.logFile, std::fstream::out | std::fstream::app);
                if(this->logFile.fail()) {
                    throw std::runtime_error("Failed to open log file " + this->config.logFile + " for resource " + this->config.resourcePath);
                }
            } else {
                throw std::runtime_error("Resource method requests logging, but resource " + this->config.resourcePath + " has no defined log file");
            }
        }
    }
}

int Resource::attach(coap_context_t *ctx) {
    coap_str_const_t *res_str;

    std::cerr << "Attaching resource " << this->config.resourcePath << std::endl;

    res_str = coap_make_str_const(this->config.resourcePath.c_str());
    this->res = coap_resource_init(res_str, 0);
    if(this->config.observable) {
        coap_resource_set_get_observable(this->res, 1);
    }

    coap_resource_set_userdata(this->res, this);
    std::list<ResourceMethod>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        switch(it->getMethodType()) {
            case ResourceMethodType::GET:
                std::cerr << "  - Attaching GET handler" << std::endl;
                coap_register_handler(this->res, COAP_REQUEST_GET, Resource::coapHandlerX<ResourceMethodType::GET>);
                break;
            case ResourceMethodType::POST:
                std::cerr << "  - Attaching POST handler" << std::endl;
                coap_register_handler(this->res, COAP_REQUEST_POST, Resource::coapHandlerX<ResourceMethodType::POST>);
                break;
            case ResourceMethodType::PUT:
                std::cerr << "  - Attaching PUT handler" << std::endl;
                coap_register_handler(this->res, COAP_REQUEST_PUT, Resource::coapHandlerX<ResourceMethodType::PUT>);
                break;
            case ResourceMethodType::DELETE:
            case ResourceMethodType::FETCH:
            case ResourceMethodType::PATCH:
            case ResourceMethodType::iPATCH:
            case ResourceMethodType::None:
                std::cerr << "ERROR: Resource::attach: Unhandled ResourceMethodType." << std::endl;
                return 1;
        }
    }
    coap_add_resource(ctx, this->res);

    this->value = this->config.initialValue;

    return 0;
}

void Resource::handleCoAPRequest(coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method) {
    std::vector<uint8_t> data;

    if(method == ResourceMethodType::PUT ||
       method == ResourceMethodType::POST) {
        int ret = this->sessionGetData(request, response, session, data);
        if(ret == 0) {
            /* Waiting on more data */
            response->code = COAP_RESPONSE_CODE_CONTINUE;
            return;
        } else if(ret < 0) {
            /* Error */
            response->code = COAP_RESPONSE_CODE_INTERNAL_ERROR;
            return;
        }
    }

    std::list<ResourceMethod>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        if(it->getMethodType() == method) {
            it->methodHandler(request, response, data, this->logFile);
            break;
        }
    }
}

void Resource::setValue(std::string _value) {
    if(this->config.observable) {
        coap_resource_notify_observers(this->res, nullptr);
    }
    this->value = _value;
}

std::string Resource::getValue() {
    return this->value;
}

std::string Resource::getPath() {
    return this->config.resourcePath;
}

unsigned Resource::getMaxAge() {
    return this->config.maxAge;
}

static void
_cache_free_app_data(void *data) {
    delete static_cast<std::vector<uint8_t> *>(data);
}

int Resource::sessionGetData(coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, std::vector<uint8_t> &dataOut) {
    (void)response;
    
    size_t         size;
    size_t         offset;
    size_t         total;
    const uint8_t *data;

    std::vector<uint8_t> *data_so_far;

    /* Handle block-wise transfers
     *   Heavily based off of https://github.com/obgm/libcoap/blob/develop/examples/coap-server.c */
    if(coap_get_data_large(request, &size, &data, &offset, &total) &&
       size != total) {
        coap_cache_entry_t *cache_entry = coap_cache_get_by_pdu(session, request, COAP_CACHE_IS_SESSION_BASED);

        if(!cache_entry) {
            if(offset != 0) {
                std::cerr << "CoAP cache entry NULL on non-first block for '" << this->getPath() << "'!" << std::endl;
                return -1;
            } else {
                cache_entry = coap_new_cache_entry(session, request, COAP_CACHE_NOT_RECORD_PDU, COAP_CACHE_IS_SESSION_BASED, 0);
                if(cache_entry == nullptr) {
                    std::cerr << "Could not create CoAP cache entry for '" << this->getPath() << "'!" << std::endl;
                    return 1;
                }
                data_so_far = new std::vector<uint8_t>;
                if(data_so_far == nullptr) {
                    std::cerr << "Could not create blockwise data buffer for '" << this->getPath() << "'!" << std::endl;
                    return -1;
                }
                coap_cache_set_app_data(cache_entry, data_so_far, _cache_free_app_data);
            }
        } else if(offset == 0) {
            data_so_far = static_cast<std::vector<uint8_t> *>(coap_cache_get_app_data(cache_entry));
            if(data_so_far) {
                data_so_far->clear();
            } else {
                data_so_far = new std::vector<uint8_t>;
                if(data_so_far == nullptr) {
                    std::cerr << "Could not create blockwise data buffer for '" << this->getPath() << "'!" << std::endl;
                    return -1;
                }
                coap_cache_set_app_data(cache_entry, data_so_far, _cache_free_app_data);
            }
        }

        if(size) {
            data_so_far = static_cast<std::vector<uint8_t> *>(coap_cache_get_app_data(cache_entry));
            /* TODO: Support re-retransmissions, if necessary. */
            data_so_far->insert(data_so_far->end(), data, data + size);
        }

        if((offset + size) == total) {
            /* We've gathered all blocks */
            data_so_far = static_cast<std::vector<uint8_t> *>(coap_cache_get_app_data(cache_entry));
            coap_cache_set_app_data(cache_entry, nullptr, nullptr);
            dataOut.clear();
            dataOut = *data_so_far;
            
            return 1;
        }
    } else {
        /* Single-block transfer */
        dataOut.clear();
        dataOut.insert(dataOut.end(), data, data + size);

        return 1;
    }

    return 0;
}


template<ResourceMethodType T>
void Resource::coapHandlerX(coap_context_t *context, coap_resource_t *resource, coap_session_t *session,
                            coap_pdu_t *request, coap_binary_t *token, coap_string_t *query,
                            coap_pdu_t *response) {
    (void)context;
    (void)resource;
    (void)session;
    (void)token;
    (void)query;

    Resource *res = static_cast<Resource *>(coap_resource_get_userdata(resource));

    res->handleCoAPRequest(request, response, session, T);
}
