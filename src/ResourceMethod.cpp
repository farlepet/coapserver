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

    std::stringstream ss;
    ss << this->methText[this->config.type];

    std::string path = this->parentResource.getPath();

    switch(this->config.type) {
        case ResourceMethodType::GET: {
            std::string val = this->parentResource.getValue();
            ss << ": " << val;
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                            coap_encode_var_safe(buf, 16, COAP_MEDIATYPE_TEXT_PLAIN), buf);
            coap_add_option(response, COAP_OPTION_MAXAGE,
                            coap_encode_var_safe(buf, 16, 60), buf);
            strncpy((char *)buf, path.c_str(), 255);
            coap_add_option(response, COAP_OPTION_URI_PATH, strlen((char *)buf), buf);

            coap_add_data(response, val.size(), (const uint8_t *)val.c_str());
            response->code = COAP_RESPONSE_CODE_CONTENT;
            response->type = COAP_MESSAGE_NON;
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST:
            if (this->fmt) {
                ss << ": ";
                this->fmt->decode(request, ss);
            }
            response->code = COAP_RESPONSE_CODE_OK;
            break;
        default:
            break;
    }

    if(ss.str().size()) {
        if(this->config.printValue) {
            std::cout << ss.str() << std::endl;
        }
        if(this->config.logValue) {
            log << ss.str() << std::endl;
        }
    }
}

ResourceMethodType ResourceMethod::getMethodType() {
    return this->config.type;
}