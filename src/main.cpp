#include <iostream>

#include "Config.hpp"
#include "CoAPServer.hpp"

int main(void) {
    Config conf("config.json");
    CoAPServer coap(conf);

    coap.start();

    return 0;
}
