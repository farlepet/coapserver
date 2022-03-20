#include <thread>
#include <atomic>
#include <iostream>
#include <signal.h>
#include <boost/program_options.hpp>

#include "Config.hpp"
#include "CoAPServer.hpp"
#include "RequestQueue.hpp"

typedef struct {
    std::string configPath;
    std::string portOverride;
    std::string addrOverride;
    std::string logDirOverride;
    size_t      queueSize;
} options_t;

static int _handleCmdLine(int, char **, options_t &);
static int _requestQueueThread(RequestQueue &);

static std::atomic<bool> _exit_now = false;

static void _signal_handler(int sig) {
    std::cerr << "Received signal " << strsignal(sig) << " - Exiting..." << std::endl;
    _exit_now = true;
}

int main(int argc, char **argv) {
    options_t options;

    int ret = _handleCmdLine(argc, argv, options);
    if(ret) {
        if (ret > 0) {
            /* Non-error exit case */
            return 0;
        } else {
            return 1;
        }
    }

    Config conf(options.configPath);

    if(options.portOverride.size()) {
        /* TODO: Check for non-numerical characters */
        int port = std::stoi(options.portOverride);
        if((port <= 0) ||
           (port >  65535)) {
            std::cerr << "Port must be an integer between 1 and 65536 (inclusive)" << std::endl;
            return 1;
        }
        conf.getEndpoint().port = options.portOverride;
    }
    if(options.addrOverride.size()) {
        /* TODO: Check address format */
        conf.getEndpoint().address = options.addrOverride;
    }
    if(options.logDirOverride.size()) {
        conf.getEndpoint().logDir = options.logDirOverride;
    }

    RequestQueue rQueue(options.queueSize);
    CoAPServer   coap(conf, rQueue);

    signal(SIGINT, _signal_handler);

    std::thread rQueueThread(_requestQueueThread, std::ref(rQueue));

    coap.init();

    while(_exit_now == false) {
        coap.exec(200);
    }

    rQueueThread.join();

    coap.exit();

    return 0;
}

static int _requestQueueThread(RequestQueue &queue) {
    while(_exit_now == false) {
        while(queue.count()) {
            RequestQueueItem item = queue.dequeue();
            item.src->handleRequestQueueItem(item);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

namespace po = boost::program_options;

static int _handleCmdLine(int argc, char **argv, options_t &_options) {
    po::options_description desc("Program options");

    /* TODO: Clean this up if more options are added. */
    desc.add_options()
        ("help,h",   "Print this help message")
        ("config,c", po::value<std::string>(&_options.configPath)->default_value("config.json"), "Configuration file")
        ("port,p",   po::value<std::string>(&_options.portOverride),                             "Override listening port")
        ("addr,a",   po::value<std::string>(&_options.addrOverride),                             "Override listening address")
        ("logdir,l", po::value<std::string>(&_options.logDirOverride),                           "Override base log directory")
        ("queue,q",  po::value<size_t>     (&_options.queueSize)->default_value(32),             "Set request handler queue size");

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
