/*  Exceptions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Exceptions.h"

namespace PokemonAutomation{



void Exception::log(Logger& logger) const{
    logger.log(std::string(name()) + ": " + message(), COLOR_RED);
}
std::string Exception::message() const{
    return "";
}
std::string Exception::to_str() const{
    std::string str = name();
    std::string msg = message();
    if (!msg.empty()){
        str += "\n\n";
        str += msg;
    }
    return str;
}




FileException::FileException(Logger* logger, const char* location, std::string message_string, std::string file)
    : m_location(location)
    , m_message(std::move(message_string))
    , m_file(std::move(file))
{
    std::string str = FileException::name();
    str += ": " + message();
    if (logger != nullptr){
        logger->log(str, COLOR_RED);
    }else{
        std::cerr << str << std::endl;
    }
}
std::string FileException::message() const{
    return m_message + "\nFile: " + m_file + "\nLocation: " + m_location;
}



ConnectionException::ConnectionException(Logger* logger, std::string message)
    : m_message(std::move(message))
{
    if (logger != nullptr){
        logger->log(std::string(ConnectionException::name()) + ": " + ConnectionException::message(), COLOR_RED);
    }else{
        std::cerr << std::string(ConnectionException::name()) + ": " + ConnectionException::message() << std::endl;
    }
}



SerialProtocolException::SerialProtocolException(Logger& logger, const char* /*location*/, std::string message)
    //: m_location(location)
    : m_message(std::move(message))
{
    logger.log(std::string(SerialProtocolException::name()) + ": " + SerialProtocolException::message(), COLOR_RED);
}
std::string SerialProtocolException::message() const{
    return m_message;
//    return m_message + "\nLocation: " + m_location;
}


InternalProgramError::InternalProgramError(Logger* logger, const char* location, std::string message)
    : m_location(location)
    , m_message(std::move(message))
{
    if (logger != nullptr){
        logger->log(std::string(InternalProgramError::name()) + ": " + InternalProgramError::message(), COLOR_RED);
    }else{
        std::cerr << std::string(InternalProgramError::name()) + ": " + InternalProgramError::message() << std::endl;
    }
}
std::string InternalProgramError::message() const{
    return m_message + "\nLocation: " + m_location + "\nPlease report this as a bug.";
}


InternalSystemError::InternalSystemError(Logger* logger, const char* location, std::string message)
    : m_location(location)
    , m_message(std::move(message))
{
    if (logger != nullptr){
        logger->log(std::string(InternalSystemError::name()) + ": " + InternalSystemError::message(), COLOR_RED);
    }else{
        std::cerr << std::string(InternalSystemError::name()) + ": " + InternalSystemError::message() << std::endl;
    }
}
std::string InternalSystemError::message() const{
    return m_message + "\nLocation: " + m_location;
}



UserSetupError::UserSetupError(Logger& logger, std::string message)
    : m_message(std::move(message))
{
    logger.log(std::string(UserSetupError::name()) + ": " + UserSetupError::message(), COLOR_RED);
}
std::string UserSetupError::message() const{
    return m_message;
}











}
