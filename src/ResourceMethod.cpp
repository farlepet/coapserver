#include <ctime>
#include <sstream>
#include <iostream>
#include <boost/process.hpp>

#include "ResourceMethod.hpp"
#include "Resource.hpp"

ResourceMethod::ResourceMethod(Resource &_parentResource, ResourceMethodConfig &_config, RequestQueue &_queue) :
config(_config),
parentResource(_parentResource),
queue(_queue) {
    std::list<std::string>::iterator it = _config.format.begin();
    for(; it != _config.format.end(); it++) {
        this->fmts.push_back(ResourceFormatter::createResourceFormatter(*it));
    }
}

std::string ResourceMethod::methStringify(ResourceMethodType type) {
    return (type == ResourceMethodType::None)   ? "None"   :
           (type == ResourceMethodType::GET)    ? "GET"    :
           (type == ResourceMethodType::POST)   ? "POST"   :
           (type == ResourceMethodType::PUT)    ? "PUT"    :
           (type == ResourceMethodType::DELETE) ? "DELETE" :
           (type == ResourceMethodType::FETCH)  ? "FETCH"  :
           (type == ResourceMethodType::PATCH)  ? "PATCH"  :
           (type == ResourceMethodType::iPATCH) ? "iPATCH" : "???";
}

int ResourceMethod::executeCmd(std::stringstream &data, std::string cmd) {
    std::string dataIn = data.str();
    /* Reset stringstream */
    data.str("");
    data.clear();

    /* TODO: Format cmd is cmdstdin is false */

    boost::process::ipstream cstdout;
    boost::process::opstream cstdin;

    boost::process::child cproc(cmd,
                                boost::process::std_out > cstdout,
                                boost::process::std_in < cstdin);

    if(this->config.cmdStdin) {
        cstdin << dataIn;
    }

    cstdin.close();
    /* Seems a little hacky, but it works: */
    cstdin.pipe().close();

    std::cerr << "[CMD] Wrote " << dataIn << " to stdin" << std::endl;

    cproc.wait();

    std::cerr << "[CMD] Command exited with result code " << cproc.exit_code() << std::endl;

    std::string line;
    while(std::getline(cstdout, line) && !line.empty()) {
        data << line;
    }

    std::cerr << "[CMD] Command Result: " << data.str() << std::endl;

    return cproc.exit_code();
}

void ResourceMethod::methodHandler(const coap_pdu_t *request, coap_pdu_t *response, std::vector<uint8_t> &data) {
    (void)request; /* Not currently used */

    time_t time = std::time(nullptr);

    switch(this->config.type) {
        case ResourceMethodType::GET: {
            std::string val  = this->parentResource.getValue();
            std::string path = this->parentResource.getPath();
            uint8_t buf[256];

            if(this->queue.enqueue(RequestQueueItem(this, this->parentResource.getValue(), time))) {
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                return;
            }

            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                            coap_encode_var_safe(buf, 16, COAP_MEDIATYPE_TEXT_PLAIN), buf);
            coap_add_option(response, COAP_OPTION_MAXAGE,
                            coap_encode_var_safe(buf, 16, this->parentResource.getMaxAge()), buf);
            strncpy(reinterpret_cast<char *>(buf), path.c_str(), 255);
            coap_add_option(response, COAP_OPTION_URI_PATH, strlen(reinterpret_cast<char *>(buf)), buf);

            coap_add_data(response, val.size(), reinterpret_cast<const uint8_t *>(val.c_str()));

            coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
            coap_pdu_set_type(response, COAP_MESSAGE_NON);
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST: {
            if(this->queue.enqueue(RequestQueueItem(this, data, time))) {
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                return;
            }

            coap_pdu_set_code(response, COAP_RESPONSE_CODE_OK);
            coap_pdu_set_type(response, COAP_MESSAGE_ACK);
        } break;
        case ResourceMethodType::DELETE:
        case ResourceMethodType::FETCH:
        case ResourceMethodType::PATCH:
        case ResourceMethodType::iPATCH:
        case ResourceMethodType::None:
            break;
    }
}

ResourceMethodType ResourceMethod::getMethodType() {
    return this->config.type;
}

int ResourceMethod::handleRequestQueueItem(RequestQueueItem &item) {
    /* TODO: Allow selecting time format */
    char       timebuf[32];
    struct tm *tm   = localtime(&item.time);
    strftime(timebuf, 32, "%Y-%m-%dT%H:%M:%S", tm);
    
    std::list<std::stringstream> sss;
    
    switch(this->config.type) {
        case ResourceMethodType::GET: {
            std::string str(item.data.begin(), item.data.end());
            sss.push_back(std::stringstream());
            sss.back() << str;
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST: {
            std::list<ResourceFormatter *>::iterator it = this->fmts.begin();
            for(; it != this->fmts.end(); it++) {
                sss.push_back(std::stringstream());
                (*it)->decode(item.data, sss.back());
                
                if(this->config.cmd.size()) {
                    this->executeCmd(sss.back(), this->config.cmd);
                }
                
                if(this->config.store &&
                   (it == this->fmts.begin())) {
                    this->parentResource.setValue(sss.back().str());
                }
            }
        } break;
        case ResourceMethodType::DELETE:
        case ResourceMethodType::FETCH:
        case ResourceMethodType::PATCH:
        case ResourceMethodType::iPATCH:
        case ResourceMethodType::None:
            break;
    }

    std::string logValue = "";
    /* TODO: Handle indentation of multi-line output, possibly print a
     * timestamp on each line. */
    logValue += timebuf;
    logValue += " | " + this->parentResource.getPath() + ":" + this->methStringify(this->config.type) + ": ";

    std::list<std::stringstream>::iterator sit = sss.begin();
    for(; sit != sss.end(); sit++) {
        if(this->config.printValue) {
            std::cout << logValue << (*sit).str() << std::endl;
        }
        if(this->config.logValue) {
            this->parentResource.writeLog(logValue + (*sit).str());
        }
    }

    return 0;
}
