#include "ResourceFormatter.hpp"
#include "ResourceFormatter/ResourceFormatterHex.hpp"
#include "ResourceFormatter/ResourceFormatterText.hpp"
#include "ResourceFormatter/ResourceFormatterMsgpack.hpp"

ResourceFormatter::ResourceFormatter() {

}

ResourceFormatter *ResourceFormatter::createResourceFormatter(ResourceMethodConfig &_config) {
    if(_config.format == "STRING") {
        return new ResourceFormatterText();
    } else if(_config.format == "HEX") {
        return new ResourceFormatterHex();
    } else if(_config.format == "MSGPACK") {
        return new ResourceFormatterMsgpack();
    } else {
        throw "Invalid resource format type";
    }
}
