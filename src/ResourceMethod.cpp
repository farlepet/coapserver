#include <ctime>
#include <sstream>
#include <iostream>
#include <boost/process.hpp>

#include "ResourceMethod.hpp"
#include "Resource.hpp"

ResourceMethod::ResourceMethod(Resource &_parentResource, const ResourceMethodConfig &_config, RequestQueue &_queue) :
config(_config),
parentResource(_parentResource),
queue(_queue) {
    std::list<std::string>::const_iterator it = _config.format.begin();
    for(; it != _config.format.end(); it++) {
        this->fmts.push_back(ResourceFormatter::createResourceFormatter(*it));
    }
}

ResourceMethod::~ResourceMethod(void) {
    std::list<ResourceFormatter*>::iterator it = this->fmts.begin();
    for(; it != this->fmts.end(); it++) {
        delete *it;
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

void ResourceMethod::methodHandler(const coap_pdu_t *request, coap_pdu_t *response, std::vector<std::byte> &data) {
    (void)request; /* Not currently used */

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    switch(this->config.type) {
        case ResourceMethodType::GET: {
            std::vector<std::byte> val = this->parentResource.getValue();
            std::string path           = this->parentResource.getPath();
            uint8_t buf[256];

            if(this->queue.enqueue(RequestQueueItem(this, this->parentResource.getValue(), tv))) {
                coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
                return;
            }

            strncpy(reinterpret_cast<char *>(buf), path.c_str(), 255);
            coap_add_option(response, COAP_OPTION_URI_PATH, strlen(reinterpret_cast<char *>(buf)), buf);
            coap_add_option(response, COAP_OPTION_CONTENT_FORMAT,
                            coap_encode_var_safe(buf, 16, COAP_MEDIATYPE_TEXT_PLAIN), buf);
            coap_add_option(response, COAP_OPTION_MAXAGE,
                            coap_encode_var_safe(buf, 16, this->parentResource.getMaxAge()), buf);

            coap_add_data(response, val.size(), reinterpret_cast<const uint8_t *>(&val[0]));

            coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST: {
            if(this->queue.enqueue(RequestQueueItem(this, data, tv))) {
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

int ResourceMethod::executeCmd(std::stringstream &data, std::string cmd) {
    std::string dataIn = data.str();

    boost::process::environment env = boost::this_process::environment();
    env["COAPSERVER_PAYLOAD"]  = dataIn;
    env["COAPSERVER_RESOURCE"] = this->parentResource.getPath();
    env["COAPSERVER_METHOD"]   = this->methStringify(this->config.type);

    boost::process::ipstream cstdout;
    boost::process::opstream cstdin;

    boost::process::child cproc(cmd,
                                boost::process::std_out > cstdout,
                                boost::process::std_in < cstdin,
                                env);

    if(this->config.cmdStdin) {
        cstdin << dataIn;
    }

    cstdin.close();
    /* Seems a little hacky, but it works: */
    cstdin.pipe().close();

    if(!cproc.wait_for(std::chrono::milliseconds{this->config.cmdTimeout})) {
        std::cerr << "Command [" << cmd << "] did not exit after " << this->config.cmdTimeout << " ms!" << std::endl;
        return -1;
    }

    /* Reset stringstream */
    data.str("");
    data.clear();

    std::string line;
    while(std::getline(cstdout, line) && !line.empty()) {
        data << line;
    }

    return cproc.exit_code();
}

int ResourceMethod::handleRequestQueueItem(RequestQueueItem &item) {
    /* TODO: Allow selecting time format */
    /* Size based on YYYY-MM-DDTHH:MM:SS.mmm plus NULL termination. */
    #define TIMEBUF_SZ 24
    char       timebuf[TIMEBUF_SZ];
    struct tm *tm   = localtime(&item.time.tv_sec);
    size_t     end  = strftime(timebuf, TIMEBUF_SZ, "%Y-%m-%dT%H:%M:%S", tm);
    snprintf(&timebuf[end], TIMEBUF_SZ - end, ".%03u", static_cast<unsigned int>(item.time.tv_usec / 1000));
    
    std::list<std::stringstream> sss;
    
    switch(this->config.type) {
        case ResourceMethodType::GET: {
            /* @todo Need to format GET data to be printable, if needed. */
            /*std::string str(item.data.begin(), item.data.end());
            sss.push_back(std::stringstream());
            sss.back() << str;*/
            std::list<ResourceFormatter *>::iterator it = this->fmts.begin();
            for(; it != this->fmts.end(); it++) {
                sss.push_back(std::stringstream());
                (*it)->decode(item.data, sss.back());

                if(this->config.cmd.size()) {
                    if(this->config.cmdLogInput) {
                        /* We want to log the input, so just duplicate the stringstream */
                        std::string dup = sss.back().str();
                        sss.push_back(std::stringstream(dup));
                    }
                    this->executeCmd(sss.back(), this->config.cmd);
                }
                
                if(this->config.store &&
                   (it == this->fmts.begin())) {
                    this->parentResource.setValue(sss.back().str());
                }
            }
        } break;
        case ResourceMethodType::PUT:
        case ResourceMethodType::POST: {
            std::list<ResourceFormatter *>::iterator it = this->fmts.begin();
            for(; it != this->fmts.end(); it++) {
                sss.push_back(std::stringstream());
                (*it)->decode(item.data, sss.back());

                if(this->config.cmd.size()) {
                    if(this->config.cmdLogInput) {
                        /* We want to log the input, so just duplicate the stringstream */
                        std::string dup = sss.back().str();
                        sss.push_back(std::stringstream(dup));
                    }
                    this->executeCmd(sss.back(), this->config.cmd);
                }
            }

            if(this->config.store) {
                /* @todo Support using output of command or decoder to set
                 * resource value. This was moved out of the ResourceFormatter
                 * loop in order to better support raw data, without needing to
                 * add encoders to each ResourceFormatter. */
                this->parentResource.setValue(item.data);
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
