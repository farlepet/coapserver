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
    ResourceMethodType type       = ResourceMethodType::None;
    /** Format of data sent to/from resource */
    std::string        format     = "STRING";
    /** Whether or not to print requests to STDOUT */
    bool               printValue = false;
    /** Whether or not to log requests to a file */
    bool               logValue   = false;
    /** Whether or not to store the received data as the resource's value */
    bool               store      = false;
    /** Command to run when a request is received */
    std::string        cmd        = "";

    ResourceMethodConfig(nlohmann::json &_json, ResourceMethodType _type);
    ResourceMethodConfig() {};
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
    ResourceConfig() {};
};

struct EndpointConfig {
    /** Address to bind on */
    std::string address = "0.0.0.0";
    /** Port to bind to */
    std::string port    = "5683";

    EndpointConfig(nlohmann::json &_json);
    EndpointConfig() {};
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
