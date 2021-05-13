#include <ctime>
#include <sstream>
#include <iostream>

#include "ResourceMethod.hpp"
#include "Resource.hpp"

ResourceMethod::ResourceMethod(Resource &_parentResource, ResourceMethodConfig &_config) :
config(_config),
parentResource(_parentResource) {
    this->fmt = ResourceFormatter::createResourceFormatter(_config);

    this->methText[ResourceMethodType::None]    = "None";
    this->methText[ResourceMethodType::GET]     = "GET";
    this->methText[ResourceMethodType::POST]    = "POST";
    this->methText[ResourceMethodType::PUT]     = "PUT";
    this->methText[ResourceMethodType::DELETE]  = "DELETE";
    this->methText[ResourceMethodType::FETCH]   = "FETCH";
    this->methText[ResourceMethodType::PATCH]   = "PATCH";
    this->methText[ResourceMethodType::iPATCH]  = "iPATCH";
}

void ResourceMethod::methodHandler(coap_pdu_t *request, coap_pdu_t *response, std::ostream &log) {
    uint8_t buf[256];
    
    /* TODO: Allow selecting time format */
    char       timebuf[32];
    time_t     time = std::time(nullptr);
    struct tm *tm   = localtime(&time);
    strftime(timebuf, 32, "%Y-%m-%dT%H:%M:%S", tm);

    std::stringstream ss;

    std::string path = this->parentResource.getPath();

    switch(this->config.type) {
        case ResourceMethodType::GET: {
            std::string val = this->parentResource.getValue();
            ss << val;
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                            coap_encode_var_safe(buf, 16, COAP_MEDIATYPE_TEXT_PLAIN), buf);
            coap_add_option(response, COAP_OPTION_MAXAGE,
                            coap_encode_var_safe(buf, 16, 1200), buf);
            strncpy((char *)buf, path.c_str(), 255);
            coap_add_option(response, COAP_OPTION_URI_PATH, strlen((char *)buf), buf);

            coap_add_data(response, val.size(), (const uint8_t *)val.c_str());
            response->code = COAP_RESPONSE_CODE_CONTENT;
            response->type = COAP_MESSAGE_NON;
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST:
            if(this->fmt) {
                this->fmt->decode(request, ss);
                if(this->config.store) {
                    this->parentResource.setValue(ss.str());
                }
            }
            response->code = COAP_RESPONSE_CODE_OK;
            break;
        default:
            break;
    }

    std::string logValue = "";
    /* TODO: Handle indentation of multi-line output, possibly print a
     * timestamp on each line. */
    logValue += timebuf;
    logValue += " | " + this->parentResource.getPath() + ":" + this->methText[this->config.type];
    if(ss.str().size()) {
        logValue += ": " + ss.str();
    }

    if(this->config.printValue) {
        std::cout << logValue << std::endl;
    }
    if(this->config.logValue) {
        log << logValue << std::endl;
    }
}

ResourceMethodType ResourceMethod::getMethodType() {
    return this->config.type;
}