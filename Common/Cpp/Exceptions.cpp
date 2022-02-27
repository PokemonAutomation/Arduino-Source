/*  Exceptions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include "Exceptions.h"

namespace PokemonAutomation{



OperationFailedException::OperationFailedException(Logger& logger, std::string message)
    : m_message(message)
{
    logger.log(std::string(name()) + ": " + m_message, COLOR_RED);
}




FileException::FileException(Logger* logger, const char* location, std::string message, std::string file)
    : m_location(location)
    , m_message(std::move(message))
    , m_file(std::move(file))
{
    std::string str = FileException::name();
    str += ": " + m_message;
    if (logger != nullptr){
        logger->log(str, COLOR_RED);
    }else{
        std::cerr << str << std::endl;
    }
}
std::string FileException::message() const{
    return m_message + "\nFile: " + m_file + "\n Location: " + m_location;
}




SerialProtocolException::SerialProtocolException(Logger& logger, std::string message)
    : m_message(std::move(message))
{
    logger.log(std::string(name()) + ": " + m_message, COLOR_RED);
}





}
