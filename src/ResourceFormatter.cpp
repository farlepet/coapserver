#include "config.h"
#include "ResourceFormatter.hpp"
#include "ResourceFormatter/ResourceFormatterHex.hpp"
#include "ResourceFormatter/ResourceFormatterText.hpp"
#ifdef USE_MSGPACK
#  include "ResourceFormatter/ResourceFormatterMsgpack.hpp"
#endif

ResourceFormatter::ResourceFormatter() {

}
        
ResourceFormatter::~ResourceFormatter() {

}

ResourceFormatter *ResourceFormatter::createResourceFormatter(std::string &_format) {
    if(_format == "STRING") {
        return new ResourceFormatterText();
    } else if(_format == "HEX") {
        return new ResourceFormatterHex();
#ifdef USE_MSGPACK
    } else if(_format == "MSGPACK") {
        return new ResourceFormatterMsgpack();
#endif
    } else {
        throw std::runtime_error("Invalid resource format type: " + _format);
    }
}
