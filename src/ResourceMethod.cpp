#include <ctime>
#include <sstream>
#include <iostream>

#include "ResourceMethod.hpp"
#include "Resource.hpp"

ResourceMethod::ResourceMethod(Resource &_parentResource, ResourceMethodConfig &_config) :
config(_config),
parentResource(_parentResource) {
    std::list<std::string>::iterator it = _config.format.begin();
    for(; it != _config.format.end(); it++) {
        this->fmts.push_back(ResourceFormatter::createResourceFormatter(*it));
    }

    this->methText[ResourceMethodType::None]    = "None";
    this->methText[ResourceMethodType::GET]     = "GET";
    this->methText[ResourceMethodType::POST]    = "POST";
    this->methText[ResourceMethodType::PUT]     = "PUT";
    this->methText[ResourceMethodType::DELETE]  = "DELETE";
    this->methText[ResourceMethodType::FETCH]   = "FETCH";
    this->methText[ResourceMethodType::PATCH]   = "PATCH";
    this->methText[ResourceMethodType::iPATCH]  = "iPATCH";
}

void ResourceMethod::methodHandler(coap_pdu_t *request, coap_pdu_t *response, std::vector<uint8_t> &data, std::ostream &log) {
    (void)request; /* Not currently used */

    uint8_t buf[256];
    
    /* TODO: Allow selecting time format */
    char       timebuf[32];
    time_t     time = std::time(nullptr);
    struct tm *tm   = localtime(&time);
    strftime(timebuf, 32, "%Y-%m-%dT%H:%M:%S", tm);

    std::list<std::stringstream> sss;

    std::string path = this->parentResource.getPath();

    switch(this->config.type) {
        case ResourceMethodType::GET: {
            std::string val = this->parentResource.getValue();
            sss.push_back(std::stringstream());
            sss.back() << val;
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                            coap_encode_var_safe(buf, 16, COAP_MEDIATYPE_TEXT_PLAIN), buf);
            coap_add_option(response, COAP_OPTION_MAXAGE,
                            coap_encode_var_safe(buf, 16, this->parentResource.getMaxAge()), buf);
            strncpy((char *)buf, path.c_str(), 255);
            coap_add_option(response, COAP_OPTION_URI_PATH, strlen((char *)buf), buf);

            coap_add_data(response, val.size(), (const uint8_t *)val.c_str());
            response->code = COAP_RESPONSE_CODE_CONTENT;
            response->type = COAP_MESSAGE_NON;
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST: {
            std::list<ResourceFormatter *>::iterator it = this->fmts.begin();
            for(; it != this->fmts.end(); it++) {
                sss.push_back(std::stringstream());
                (*it)->decode(data, sss.back());
                if(this->config.store &&
                   (it == this->fmts.begin())) {
                    this->parentResource.setValue(sss.back().str());
                }
            }
            response->code = COAP_RESPONSE_CODE_OK;
        } break;
        default:
            break;
    }

    std::string logValue = "";
    /* TODO: Handle indentation of multi-line output, possibly print a
     * timestamp on each line. */
    logValue += timebuf;
    logValue += " | " + this->parentResource.getPath() + ":" + this->methText[this->config.type] + ": ";
    
    std::list<std::stringstream>::iterator sit = sss.begin();
    for(; sit != sss.end(); sit++) {
        if(this->config.printValue) {
            std::cout << logValue << (*sit).str() << std::endl;
        }
        if(this->config.logValue) {
            log << logValue << (*sit).str() << std::endl;
        }
    }
}

ResourceMethodType ResourceMethod::getMethodType() {
    return this->config.type;
}