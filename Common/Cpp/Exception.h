/*  Exception
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_Exception_H
#define PokemonAutomation_Exception_H

#include <string>
#include <exception>

#ifdef QT_VERSION
#include <QString>
#endif

#include "Common/Compiler.h"

namespace PokemonAutomation{


class StringException : public std::exception{
public:
    StringException(const char* location, const char* message)
        : StringException("StringException", location, std::string(message))
    {}
    StringException(const char* location, const std::string& message)
        : StringException("StringException", location, message)
    {}
#ifdef QT_VERSION
    StringException(const char* location, const QString& message)
        : StringException("StringException", location, message.toUtf8().data())
    {}
#endif

    virtual const char* type() const{
        return "StringException";
    }
    virtual const char* what() const noexcept{
        return m_full_body.c_str();
    }
    const std::string& message() const{
        return m_message;
    }
#ifdef QT_VERSION
    QString message_qt() const{
        return QString::fromUtf8(m_message.c_str());
    }
#endif


protected:
    StringException(const char* type, const char* location, std::string message);


protected:
    std::string m_full_body;
    std::string m_message;
};
#define PA_THROW_StringException(message)   \
    throw StringException(__PRETTY_FUNCTION__, message)



class ParseException : public StringException{
public:
    using StringException::StringException;

    virtual const char* type() const{
        return "ParseException";
    }
};
#define PA_THROW_ParseException(message)    \
    throw ParseException(__PRETTY_FUNCTION__, message)



class FileException : public StringException{
public:
    FileException(const char* location, std::string message, const std::string& file);
#ifdef QT_VERSION
    FileException(const char* location, std::string message, const QString& file)
        : FileException(location, std::move(message), std::string(file.toUtf8().data()))
    {}
#endif

    virtual const char* type() const{
        return "FileException";
    }

private:
};
#define PA_THROW_FileException(message, file) \
    throw FileException(__PRETTY_FUNCTION__, message, file)


}
#endif

