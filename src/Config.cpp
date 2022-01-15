#include <fstream>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "Config.hpp"

Config::Config(std::string path) {
    std::ifstream in(path);
    if(in.fail()) {
        throw std::runtime_error("Could not open config file '" + path + "' for reading!");
    }

    in >> this->config;

    std::list<TemplateConfig>   templates;
    std::list<TemplateInstance> templateInstances;

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
    
    if(this->config.contains("templates") && this->config["templates"].is_array()) {
        nlohmann::json jsonTemplates = this->config["templates"];
        nlohmann::json::iterator it;
        for(it = jsonTemplates.begin(); it != jsonTemplates.end(); it++) {
            templates.push_back(TemplateConfig(*it));
        }
    }
    
    if(this->config.contains("template-instances") && this->config["template-instances"].is_array()) {
        nlohmann::json jsonInstances = this->config["template-instances"];
        nlohmann::json::iterator it;
        for(it = jsonInstances.begin(); it != jsonInstances.end(); it++) {
            templateInstances.push_back(TemplateInstance(*it));
        }
    }

    if(templateInstances.size()) {
        std::list<TemplateInstance>::iterator it = templateInstances.begin();
        for(; it != templateInstances.end(); it++) {
            std::list<TemplateConfig>::iterator tit = templates.begin();
            for(; tit != templates.end(); tit++) {
                if(tit->templateIdent == it->templateId) {
                    tit->GenerateResources(this->resources, *it);
                    break;
                }
            }
            if (tit == templates.end()) {
                throw std::runtime_error("Could not find desired template identifier: " + it->templateId);
            }
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
    if(_json.contains("dynamic") && _json["dynamic"].is_boolean()) {
        this->dynamic = _json["dynamic"];
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
    if(_json.contains("format")) {
        if(_json["format"].is_string()) {
            this->format.push_back(_json["format"]);
        } else if(_json["format"].is_array()) {
            nlohmann::json formats = _json["format"];
            nlohmann::json::iterator it;
            for(it = formats.begin(); it != formats.end(); it++) {
                if(it->is_string()) {
                    this->format.push_back(*it);        
                } else {
                    throw std::runtime_error("ERROR: Resource method format is of incorrect type");
                }
            }
        } else {
            throw std::runtime_error("ERROR: Resource method format is of incorrect type");
        }
    }
    if(_json.contains("print") && _json["print"].is_boolean()) {
        this->printValue = _json["print"];
    }
    if(_json.contains("log") && _json["log"].is_boolean()) {
        this->logValue = _json["log"];
    }
    if(_json.contains("store") && _json["store"].is_boolean()) {
        this->store = _json["store"];
    }
    if(_json.contains("cmd") && _json["cmd"].is_string()) {
        this->cmd = _json["cmd"];
    }
}

TemplateInstance::TemplateInstance(nlohmann::json &_json) {
    if(_json.contains("template") && _json["template"].is_string()) {
        this->templateId = _json["template"];
    } else {
        throw std::runtime_error("ERROR: Template instance does not include template identifier!");
    }
    
    if(_json.contains("template-args") && _json["template-args"].is_array()) {
        nlohmann::json args = _json["template-args"];
        nlohmann::json::iterator it;
        for(it = args.begin(); it != args.end(); it++) {
            if(it->is_string()) {
                this->templateArgs.push_back(*it);
            } else {
                throw std::runtime_error("ERROR: Template instance argument is not a string!");
            }
        }
    } else {
        throw std::runtime_error("ERROR: Template instance does not include arguemnts!");
    }
}

TemplateConfig::TemplateConfig(nlohmann::json &_json) {
    if(_json.contains("name") && _json["name"].is_string()) {
        this->templateName = _json["name"];
    }

    if(_json.contains("ident") && _json["ident"].is_string()) {
        this->templateIdent = _json["ident"];
    } else {
        throw std::runtime_error("ERROR: Template config does not include identifier!");
    }

    if(_json.contains("resources") && _json["resources"].is_array()) {
        nlohmann::json _resources = _json["resources"];
        nlohmann::json::iterator it;
        for(it = _resources.begin(); it != _resources.end(); it++) {
            if(it->is_object()) {
                this->resources.push_back(ResourceConfig(*it));
            } else {
                throw std::runtime_error("ERROR: Template resource config item is not an object!");
            }
        }
    } else {
        throw std::runtime_error("ERROR: Template config does not contain resources!");
    }
}

static std::string _TemplateReplace(std::string in, TemplateInstance &_instance) {
    std::ostringstream out;
    for(size_t i = 0; i < in.size(); i++) {
        if(in[i] == '$') {
            i++;
            if(in[i] == '$') {
                out << in[i];
            } else if(isdigit(in[i])) {
                size_t idx = 0;
                while(isdigit(in[i])) {
                    idx = (idx * 10) + static_cast<size_t>(in[i] - '0');
                    i++;
                }
                i--;
                if(idx > _instance.templateArgs.size()) {
                    throw std::runtime_error("ERROR: Template instance does not have enough arguments for template string: `" + in + "`!");
                } else if(idx == 0) {
                    throw std::runtime_error("ERROR: Template string argument specifier cannot be zero: `" + in + "`!");
                } else {
                    out << *std::next(_instance.templateArgs.begin(), static_cast<ptrdiff_t>(idx) - 1);
                }
            } else {
                throw std::runtime_error("ERROR: Improperly formatted template string: `" + in + "`!");
            }
        } else {
            out << in[i];
        }
    }
    return out.str();
}

void TemplateConfig::GenerateResources(std::list<ResourceConfig> &_resources, TemplateInstance &_instance) {
    std::list<ResourceConfig>::iterator it = this->resources.begin();
    for(; it != this->resources.end(); it++) {
        ResourceConfig res;
        res.resourceName = _TemplateReplace(it->resourceName, _instance);
        res.resourcePath = _TemplateReplace(it->resourcePath, _instance);
        res.logFile      = _TemplateReplace(it->logFile,      _instance);
        res.initialValue = _TemplateReplace(it->initialValue, _instance);
        res.maxAge       = it->maxAge;
        res.observable   = it->observable;

        std::list<ResourceMethodConfig>::iterator mit = it->methods.begin();
        for(; mit != it->methods.end(); mit++) {
            ResourceMethodConfig meth;
            meth.type       = mit->type;
            std::list<std::string>::iterator fit = mit->format.begin();
            for(; fit != mit->format.end(); fit++) {
                meth.format.push_back(_TemplateReplace(*fit, _instance));
            }
            meth.printValue = mit->printValue;
            meth.logValue   = mit->logValue;
            meth.store      = mit->store;
            meth.cmd        = _TemplateReplace(mit->cmd, _instance);

            res.methods.push_back(meth);
        }

        _resources.push_back(res);
    }
}
