#include <iostream>
#include <filesystem>
#include <algorithm>

#include "Resource.hpp"

#include "coap3/coap.h"

Resource::Resource(const ResourceConfig *_config, RequestQueue &_queue, std::string &_logDir) :
queue(_queue) {
    this->config = _config;
    const std::list<ResourceMethodConfig> &methodConfigs = this->config->methods;

    std::list<ResourceMethodConfig>::const_iterator it = methodConfigs.begin();
    for(; it != methodConfigs.end(); it++) {
        this->methods.push_back(new ResourceMethod(*this, *it, this->queue));
        if(it->logValue && !this->logFile.is_open()) {
            if(this->config->logFile.size()) {
                /* Prepend configured log directory, if present. */
                std::string logDir;
                if(_logDir.size()) {
                    logDir = _logDir + std::filesystem::path::preferred_separator + this->config->logFile;
                } else {
                    logDir = this->config->logFile;
                }

                if(logDir.find(std::filesystem::path::preferred_separator) != std::string::npos) {
                    /* File is nested in a directory, make sure it exists. */
                    std::filesystem::create_directories(std::filesystem::path(logDir).parent_path());
                }

                this->logFile.open(logDir, std::fstream::out | std::fstream::app);
                if(this->logFile.fail()) {
                    throw std::runtime_error("Failed to open log file `" + logDir + "` for resource " + this->config->resourcePath);
                }
            } else {
                throw std::runtime_error("Resource method requests logging, but resource " + this->config->resourcePath + " has no defined log file");
            }
        }
    }

    if(this->config->dataFile.size()) {
        this->dataFilePath      = this->config->dataFile;
    }
}

Resource::~Resource(void) {
    std::list<ResourceMethod *>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        delete *it;
    }

    /* This config was dynamically allocated by CoAPServer.
     * Not the cleanest way to handle this. */
    if(this->config->dynamic) {
        delete this->config;
    }
}

int Resource::attach(coap_context_t *ctx) {
    coap_str_const_t *res_str;

    std::cerr << "Attaching resource " << this->config->resourcePath << std::endl;

    res_str = coap_make_str_const(this->config->resourcePath.c_str());
    this->res = coap_resource_init(res_str, COAP_RESOURCE_FLAGS_NOTIFY_CON);
    if(this->config->observable) {
        coap_resource_set_get_observable(this->res, 1);
    }

    coap_resource_set_userdata(this->res, this);
    std::list<ResourceMethod *>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        switch((*it)->getMethodType()) {
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

    if(!this->dataFilePath.empty() &&
       std::filesystem::exists(this->dataFilePath)) {
        /* Set last update time to an hour ago to force a read. */
        this->dataFileWriteTime = std::filesystem::last_write_time(this->dataFilePath) - std::chrono::hours(1);
        this->valueUpdate();
    } else {
        this->setValue(this->config->initialValue);
    }

    return 0;
}

void Resource::handleCoAPRequest(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, ResourceMethodType method) {
    std::vector<std::byte> data;

    if(method == ResourceMethodType::PUT ||
       method == ResourceMethodType::POST) {
        int ret = this->sessionGetData(request, response, session, data);
        if(ret == 0) {
            /* Waiting on more data */
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTINUE);
            return;
        } else if(ret < 0) {
            /* Error */
            coap_pdu_set_code(response, COAP_RESPONSE_CODE_INTERNAL_ERROR);
            return;
        }
    }

    std::list<ResourceMethod *>::iterator it = this->methods.begin();
    for(; it != this->methods.end(); it++) {
        if((*it)->getMethodType() == method) {
            (*it)->methodHandler(request, response, data);
            break;
        }
    }
}

void Resource::setValue(const std::string &_value) {
    std::vector<std::byte> vec;
    vec.reserve(_value.size());

    std::transform(_value.begin(), _value.end(), std::back_inserter(vec), [](char c) {
        return std::byte(c);
    });

    this->setValue(vec);
}

void Resource::setValue(const std::vector<std::byte> &_value) {
    this->valueLock.lock();
    this->value = _value;
    this->valueLock.unlock();

    if(this->config->observable) {
        coap_resource_notify_observers(this->res, nullptr);
    }

    if(!this->dataFilePath.empty()) {
        this->dataFileLock.lock();
        std::ofstream file(this->dataFilePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file.write(reinterpret_cast<const char *>(_value.data()), this->value.size());
        file.close();

        this->dataFileWriteTime = std::filesystem::last_write_time(this->dataFilePath);

        this->dataFileLock.unlock();
    }
}

std::vector<std::byte> Resource::getValue() {
    this->valueLock.lock();
    std::vector<std::byte> val = this->value;
    this->valueLock.unlock();
    return val;
}

void Resource::valueUpdate() {
    if(this->dataFilePath.empty() ||
       !std::filesystem::exists(this->dataFilePath)) {
        /* No file to read */
        return;
    }

    std::filesystem::file_time_type mtime = std::filesystem::last_write_time(this->dataFilePath);
    if(mtime <= this->dataFileWriteTime) {
        /* File has not been updated since it was last read. */
        return;
    }

    /* File has been modified, read it. */
    this->dataFileLock.lock();

    uintmax_t fileSize = std::filesystem::file_size(this->dataFilePath);
    std::vector<std::byte> data(fileSize);

    std::ifstream file(this->dataFilePath, std::ios::in | std::ios::binary);
    file.read(reinterpret_cast<char *>(data.data()), fileSize);
    file.close();

    /* Not using setValue, since that will attempt to write to the dataFile (it
     * may make sense to move that logic here as well). */
    this->valueLock.lock();
    this->value = data;
    this->valueLock.unlock();

    if(this->config->observable) {
        coap_resource_notify_observers(this->res, nullptr);
    }

    this->dataFileWriteTime = mtime;

    /* Waiting to do the unlock here, so an asynchronous call to setValue does
     * not mess anything up. */
    this->dataFileLock.unlock();
}

std::string Resource::getPath() {
    return this->config->resourcePath;
}

unsigned Resource::getMaxAge() {
    return this->config->maxAge;
}

static void
_cache_free_app_data(void *data) {
    delete static_cast<std::vector<uint8_t> *>(data);
}

int Resource::sessionGetData(const coap_pdu_t *request, coap_pdu_t *response, coap_session_t *session, std::vector<std::byte> &dataOut) {
    (void)response;
    
    size_t         size;
    size_t         offset;
    size_t         total;
    const uint8_t *data;

    std::vector<std::byte> *data_so_far;

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
                    return -1;
                }
                data_so_far = new std::vector<std::byte>;
                if(data_so_far == nullptr) {
                    std::cerr << "Could not create blockwise data buffer for '" << this->getPath() << "'!" << std::endl;
                    return -1;
                }
                coap_cache_set_app_data(cache_entry, data_so_far, _cache_free_app_data);
            }
        } else if(offset == 0) {
            data_so_far = static_cast<std::vector<std::byte> *>(coap_cache_get_app_data(cache_entry));
            if(data_so_far) {
                data_so_far->clear();
            } else {
                data_so_far = new std::vector<std::byte>;
                if(data_so_far == nullptr) {
                    std::cerr << "Could not create blockwise data buffer for '" << this->getPath() << "'!" << std::endl;
                    return -1;
                }
                coap_cache_set_app_data(cache_entry, data_so_far, _cache_free_app_data);
            }
        }

        if(size) {
            data_so_far = static_cast<std::vector<std::byte> *>(coap_cache_get_app_data(cache_entry));
            if(data_so_far == nullptr) {
                /* TODO: Find the cause of this occasional error */
                std::cerr << "data_so_far NULL on block for '" << this->getPath() << "'!" << std::endl;
                return -1;
            }
            /* Insert the received data at the correct location within the buffer */
            data_so_far->resize(total);
            std::copy(reinterpret_cast<const std::byte *>(data), reinterpret_cast<const std::byte *>(data + size),
                      data_so_far->begin() + static_cast<long>(offset));
        }

        if((offset + size) == total) {
            /* We've gathered all blocks */
            data_so_far = static_cast<std::vector<std::byte> *>(coap_cache_get_app_data(cache_entry));
            coap_cache_set_app_data(cache_entry, nullptr, nullptr);
            dataOut.clear();
            
            if(data_so_far == nullptr) {
                std::cerr << "data_so_far NULL on block for '" << this->getPath() << "'!" << std::endl;
                return -1;
            }
            dataOut = *data_so_far;
            delete data_so_far;
            
            return 1;
        }
    } else {
        /* Single-block transfer */
        dataOut.clear();
        dataOut.insert(dataOut.end(), reinterpret_cast<const std::byte *>(data), reinterpret_cast<const std::byte *>(data + size));

        return 1;
    }

    return 0;
}

void Resource::writeLog(const std::string &str) {
    this->logFile << str << std::endl;
}


template<ResourceMethodType T>
void Resource::coapHandlerX(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request,
                            const coap_string_t *query, coap_pdu_t *response) {
    (void)query;

    Resource *res = static_cast<Resource *>(coap_resource_get_userdata(resource));

    res->handleCoAPRequest(request, response, session, T);
}
