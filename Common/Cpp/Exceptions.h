/*  Exceptions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This the new exceptions header.
 *
 */

#ifndef PokemonAutomation_Exceptions_H
#define PokemonAutomation_Exceptions_H

#include "AbstractLogger.h"

namespace PokemonAutomation{


//  Definitions:
//      Catch:   To catch the exception using a try-catch.
//      Consume: To catch the exception and not rethrow it.



//  Base class. Don't use this directly. This is for the infra to catch everything.
class Exception{
public:
    virtual const char* name() const = 0;
    virtual std::string message() const{ return ""; };
    virtual std::string to_str() const{
        std::string str = name();
        std::string msg = message();
        if (!msg.empty()){
            str += "\n\n";
            str += msg;
        }
        return str;
    }
};


//  Thrown when the user stops the program.
//    - This should not be consumed except by the infra.
//    - Non-infra are allowed to catch and rethrow this exception.
class ProgramCancelledException : public Exception{
public:
    virtual const char* name() const override{ return "ProgramCancelledException"; }
};


//  Thrown by BotBase connections when a command is issued while the connection
//  is in a state that isn't accepting commands.
//    - This should not be consumed except by the infra.
//    - Non-infra are allowed to catch and rethrow this exception.
class InvalidConnectionStateException : public Exception{
public:
    virtual const char* name() const override{ return "InvalidConnectionStateException"; }
};


//  Thrown when a local operation is cancelled.
//  This can be caught by local handlers that do async-cancel.
//  If this propagates up to the infra, it is considered an error.
class OperationCancelledException : public Exception{
public:
    virtual const char* name() const override{ return "OperationCancelledException"; }
};


//  Thrown by subroutines if they fail for an in-game reason.
//  These are generally recoverable errors and should be consumed by the program.
class OperationFailedException : public Exception{
public:
//    OperationFailedException(std::string message) : m_message(message) {}
    OperationFailedException(Logger& logger, std::string message);
    virtual const char* name() const override{ return "OperationFailedException"; }
    virtual std::string message() const override{ return m_message; }
private:
    std::string m_message;
};





class ParseException : public Exception{
public:
    ParseException(std::string message) : m_message(std::move(message)) {}
    virtual const char* name() const override{ return "ParseException"; }
    virtual std::string message() const override{ return m_message; }
private:
    std::string m_message;
};


class FileException : public Exception{
public:
    FileException(Logger* logger, const char* location, std::string message, std::string file);
    virtual const char* name() const override{ return "FileException"; }
    virtual std::string message() const override;
private:
    const char* m_location;
    std::string m_message;
    std::string m_file;
};


class SerialProtocolException : public Exception{
public:
    SerialProtocolException(Logger& logger, std::string message);
    virtual const char* name() const override{ return "SerialProtocolException"; }
    virtual std::string message() const override{ return m_message; }
private:
    std::string m_message;
};




}
#endif
