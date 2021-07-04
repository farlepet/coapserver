#include <iostream>
#include <boost/program_options.hpp>

#include "Config.hpp"
#include "CoAPServer.hpp"

static struct {
    std::string configPath;
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
        ("config,c", po::value<std::string>(&_options.configPath)->default_value("config.json"), "Configuration file");

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
