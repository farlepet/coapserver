#include <iostream>
#include <boost/program_options.hpp>

#include "Config.hpp"
#include "CoAPServer.hpp"

static struct {
    std::string configPath;
    std::string portOverride;
    std::string addrOverride;
} _options;

static int _handleCmdLine(int, char **);

int main(int argc, char **argv) {
    int ret = _handleCmdLine(argc, argv);
    if(ret) {
        if (ret > 0) {
            /* Non-error exit case */
            return 0;
        } else {
            return 1;
        }
    }

    Config conf(_options.configPath);

    if(_options.portOverride.size()) {
        /* TODO: Check for non-numerical characters */
        int port = std::stoi(_options.portOverride);
        if((port <= 0) ||
           (port >  65535)) {
            std::cerr << "Port must be an integer between 1 and 65536 (inclusive)" << std::endl;
            return 1;
        }
        conf.getEndpoint().port = _options.portOverride;
    }
    if(_options.addrOverride.size()) {
        /* TODO: Check address format */
        conf.getEndpoint().address = _options.addrOverride;
    }

    CoAPServer coap(conf);

    coap.start();

    return 0;
}

namespace po = boost::program_options;

static int _handleCmdLine(int argc, char **argv) {
    po::options_description desc("Program options");

    /* TODO: Clean this up if more options are added. */
    desc.add_options()
        ("help,h",   "Print this help message")
        ("config,c", po::value<std::string>(&_options.configPath)->default_value("config.json"), "Configuration file")
        ("port,p",   po::value<std::string>(&_options.portOverride),                             "Override listening port")
        ("addr,a",   po::value<std::string>(&_options.addrOverride),                             "Override listening address");

    po::variables_map vmap;
    po::store(po::parse_command_line(argc, argv, desc), vmap);
    po::notify(vmap);

    if(vmap.count("help")) {
        std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    return 0;
}
