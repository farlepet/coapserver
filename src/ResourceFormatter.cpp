#include "ResourceFormatter.hpp"
#include "ResourceFormatter/ResourceFormatterHex.hpp"
#include "ResourceFormatter/ResourceFormatterText.hpp"
#include "ResourceFormatter/ResourceFormatterMsgpack.hpp"

ResourceFormatter::ResourceFormatter() {

}
        
ResourceFormatter::~ResourceFormatter() {

}

ResourceFormatter *ResourceFormatter::createResourceFormatter(std::string &_format) {
    if(_format == "STRING") {
        return new ResourceFormatterText();
    } else if(_format == "HEX") {
        return new ResourceFormatterHex();
    } else if(_format == "MSGPACK") {
        return new ResourceFormatterMsgpack();
    } else {
        throw std::runtime_error("Invalid resource format type: " + _format);
    }
}
