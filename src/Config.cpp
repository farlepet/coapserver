#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "Config.hpp"

Config::Config(std::string path) {
    std::ifstream in(path);
    if(in.fail()) {
        throw std::runtime_error("Could not open config file " + path + "for reading!");
    }

    in >> this->config;

    if(this->config.contains("endpoint") && this->config["endpoint"].is_object()) {
        this->endpoint = EndpointConfig(this->config["endpoint"]);
    }
    
    if(this->config.contains("resources") && this->config["resources"].is_array()) {
        nlohmann::json jsonResources = this->config["resources"];
        nlohmann::json::iterator it;
        for(it = jsonResources.begin(); it != jsonResources.end(); it++) {
            this->resources.push_back(ResourceConfig(*it));
        }
    }
}

EndpointConfig &Config::getEndpoint() {
    return this->endpoint;
}

std::list<ResourceConfig> &Config::getResources() {
    return this->resources;
}


EndpointConfig::EndpointConfig(nlohmann::json &_json) {
    if(_json.contains("address") && _json["address"].is_string()) {
        this->address = _json["address"];
    }
    if(_json.contains("port") && _json["port"].is_string()) {
        this->port = _json["port"];
    }
}

ResourceConfig::ResourceConfig(nlohmann::json &_json) {
    if(_json.contains("name") && _json["name"].is_string()) {
        this->resourceName = _json["name"];
    }
    if(_json.contains("path") && _json["path"].is_string()) {
        this->resourcePath = _json["path"];
    } else {
        throw std::runtime_error("ERROR: Resource does not include resource path!");
    }
    if(_json.contains("value") && _json["value"].is_string()) {
        this->initialValue = _json["value"];
    }
    if(_json.contains("logfile") && _json["logfile"].is_string()) {
        this->logFile = _json["logfile"];
    }
    if(_json.contains("observable") && _json["observable"].is_boolean()) {
        this->observable = _json["observable"];
    }
    if(_json.contains("maxage") && _json["maxage"].is_number()) {
        this->maxAge = _json["maxage"];
    }

    if(_json.contains("methods") && _json["methods"].is_object()) {
        /* TODO: Create a map to avoid code duplication */
        if(_json["methods"].contains("GET")) {
            this->methods.push_back(ResourceMethodConfig(_json["methods"]["GET"], ResourceMethodType::GET));
        }
        if(_json["methods"].contains("POST")) {
            this->methods.push_back(ResourceMethodConfig(_json["methods"]["POST"], ResourceMethodType::POST));
        }
        if(_json["methods"].contains("PUT")) {
            this->methods.push_back(ResourceMethodConfig(_json["methods"]["PUT"], ResourceMethodType::PUT));
        }
    } else {
        throw std::runtime_error("ERROR: Resource does not include any methods!");
    }
}

ResourceMethodConfig::ResourceMethodConfig(nlohmann::json &_json, ResourceMethodType _type) {
    this->type = _type;
    if(_json.contains("format") && _json["format"].is_string()) {
        this->format = _json["format"];
    }
    if(_json.contains("print") && _json["print"].is_boolean()) {
        this->printValue = _json["print"];
    }
    if(_json.contains("log") && _json["log"].is_boolean()) {
        this->logValue = _json["log"];
    }
    if(_json.contains("cmd") && _json["cmd"].is_string()) {
        this->cmd = _json["cmd"];
    }
}
