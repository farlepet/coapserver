#include <iostream>

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

    coap_resource_set_userdata(this->res, (void *)this);
    std::list<ResourceMethod>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        switch(it->getMethodType()) {
            case ResourceMethodType::GET:
                std::cerr << "  - Attaching GET handler" << std::endl;
                coap_register_handler(this->res, COAP_REQUEST_GET, Resource::coapHandlerGet);
                break;
            case ResourceMethodType::POST:
                std::cerr << "  - Attaching POST handler" << std::endl;
                coap_register_handler(this->res, COAP_REQUEST_POST, Resource::coapHandlerPost);
                break;
            case ResourceMethodType::PUT:
                std::cerr << "  - Attaching PUT handler" << std::endl;
                coap_register_handler(this->res, COAP_REQUEST_PUT, Resource::coapHandlerPut);
                break;
            default:
                std::cerr << "ERROR: Resource::attach: Unhandled ResourceMethodType." << std::endl;
                return 1;
        }
    }
    coap_add_resource(ctx, this->res);

    this->value = this->config.initialValue;

    return 0;
}

void Resource::handleCoAPRequest(coap_pdu_t *request, coap_pdu_t *response, ResourceMethodType method) {
    /* TODO: This is not a great way to do this */
    std::list<ResourceMethod>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        if(it->getMethodType() == method) {
            it->methodHandler(request, response, this->logFile);
            break;
        }
    }
}

void Resource::setValue(std::string _value) {
    if(this->config.observable) {
        coap_resource_notify_observers(this->res, NULL);
    }
    this->value = _value;
}

std::string Resource::getValue() {
    return this->value;
}

std::string Resource::getPath() {
    return this->config.resourcePath;
}

void Resource::coapHandlerGet(coap_context_t *context, coap_resource_t *resource, coap_session_t *session,
                              coap_pdu_t *request, coap_binary_t *token, coap_string_t *query,
                              coap_pdu_t *response) {
    (void)context;
    (void)resource;
    (void)session;
    (void)token;
    (void)query;

    Resource *res = (Resource *)coap_resource_get_userdata(resource);

    res->handleCoAPRequest(request, response, ResourceMethodType::GET);
}

void Resource::coapHandlerPost(coap_context_t *context, coap_resource_t *resource, coap_session_t *session,
                               coap_pdu_t *request, coap_binary_t *token, coap_string_t *query,
                               coap_pdu_t *response) {
    (void)context;
    (void)resource;
    (void)session;
    (void)token;
    (void)query;

    Resource *res = (Resource *)coap_resource_get_userdata(resource);

    res->handleCoAPRequest(request, response, ResourceMethodType::POST);
}

void Resource::coapHandlerPut(coap_context_t *context, coap_resource_t *resource, coap_session_t *session,
                              coap_pdu_t *request, coap_binary_t *token, coap_string_t *query,
                              coap_pdu_t *response) {
    (void)context;
    (void)resource;
    (void)session;
    (void)token;
    (void)query;

    Resource *res = (Resource *)coap_resource_get_userdata(resource);

    res->handleCoAPRequest(request, response, ResourceMethodType::PUT);
}
