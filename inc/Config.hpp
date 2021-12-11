#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_

#include <string>
#include <list>

#include <nlohmann/json.hpp>

enum class ResourceMethodType {
    None = 0,
    GET,
    POST,
    PUT,
    DELETE,
    FETCH,
    PATCH,
    iPATCH
};

struct ResourceMethodConfig {
    /** Resource request method type */
    ResourceMethodType     type       = ResourceMethodType::None;
    /** Format(s) of data sent to/from resource */
    std::list<std::string> format;
    /** Whether or not to print requests to STDOUT */
    bool                   printValue = false;
    /** Whether or not to log requests to a file */
    bool                   logValue   = false;
    /** Whether or not to store the received data as the resource's value */
    bool                   store      = false;
    /** Command to run when a request is received */
    std::string            cmd        = "";

    ResourceMethodConfig(nlohmann::json &_json, ResourceMethodType _type);
    ResourceMethodConfig() {}
};

struct ResourceConfig {
    /** Name of resource */
    std::string  resourceName = "";
    /** Path of resource */
    std::string  resourcePath = "";
    /** File in which to log requests to the resource */
    std::string  logFile      = "";
    /** Initial value to the resource returned by GET requests */
    std::string  initialValue = "";
    /** Max-age value */
    unsigned int maxAge       = 3600;
    /** Whether or not to allow observation of the resource */
    bool         observable   = false;

    std::list<ResourceMethodConfig> methods;

    ResourceConfig(nlohmann::json &_json);
    ResourceConfig() {}
};

struct TemplateInstance {
    /** Identifier of template to use */
    std::string templateId = "";
    /** Arguments to give template */
    std::list<std::string> templateArgs;

    TemplateInstance(nlohmann::json &_json);
    TemplateInstance() {}
};

struct TemplateConfig {
    /** Name of template */
    std::string templateName  = "";
    /** Identifier of template, used in resource */
    std::string templateIdent = "";

    /** Resource templates */
    std::list<ResourceConfig> resources;

    /** Argument names */
    std::list<std::string>    args;

    TemplateConfig(nlohmann::json &_json);
    TemplateConfig() {}

    void GenerateResources(std::list<ResourceConfig> &_resources, TemplateInstance &_instance);
};

struct EndpointConfig {
    /** Address to bind on */
    std::string address = "0.0.0.0";
    /** Port to bind to */
    std::string port    = "5683";

    EndpointConfig(nlohmann::json &_json);
    EndpointConfig() {}
};

class Config {
    private:
        nlohmann::json config;

        EndpointConfig            endpoint;
        std::list<ResourceConfig> resources;

    public:
        Config(std::string path);

        EndpointConfig &getEndpoint();

        std::list<ResourceConfig> &getResources();
};

#endif /* _CONFIG_HPP_ */
