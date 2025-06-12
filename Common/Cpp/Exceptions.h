/*  Exceptions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Exceptions_H
#define PokemonAutomation_Exceptions_H

#include <utility>
#include "Common/Compiler.h"
#include "AbstractLogger.h"

namespace PokemonAutomation{


template <typename ExceptionType, class... Args>
[[noreturn]] void throw_and_log(Logger& logger, Args&&... args){
    ExceptionType exception(std::forward<Args>(args)...);
    exception.log(logger);
    throw exception;
}


//  Definitions:
//      Catch:   To catch the exception using a try-catch.
//      Consume: To catch the exception and not rethrow it.


//  Base class. Don't use this directly. This is for the infra to catch everything.
class Exception{
public:
    virtual ~Exception() = default;

    virtual void log(Logger& logger) const;
    virtual const char* name() const = 0;
    virtual std::string message() const;
    virtual std::string to_str() const;
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
    InvalidConnectionStateException(std::string message) : m_message(std::move(message)) {}
    virtual const char* name() const override{ return "InvalidConnectionStateException"; }
    virtual std::string message() const override{ return m_message; }
protected:
    std::string m_message;
};


//  Thrown when a local operation is cancelled.
//  This can be caught by local handlers that do async-cancel.
//  If this propagates up to the infra, it is considered an error.
class OperationCancelledException : public Exception{
public:
    virtual const char* name() const override{ return "OperationCancelledException"; }
};





class ParseException : public Exception{
public:
    ParseException() = default;
    ParseException(std::string message) : m_message(std::move(message)) {}
    virtual const char* name() const override{ return "ParseException"; }
    virtual std::string message() const override{ return m_message; }
protected:
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


class ConnectionException : public Exception{
public:
    ConnectionException(Logger* logger, std::string message);
    virtual const char* name() const override{ return "ConnectionException"; }
    virtual std::string message() const override{ return m_message; }
private:
    std::string m_message;
};


class SerialProtocolException : public Exception{
public:
    SerialProtocolException(Logger& logger, const char* location, std::string message);
    virtual const char* name() const override{ return "SerialProtocolException"; }
    virtual std::string message() const override;
private:
    // const char* m_location;
    std::string m_message;
};


//  These are thrown for logic errors. They are always bugs.
class InternalProgramError : public Exception{
public:
    InternalProgramError(Logger* logger, const char* location, std::string message);
    virtual const char* name() const override{ return "InternalProgramError"; }
    virtual std::string message() const override;
private:
    const char* m_location;
    std::string m_message;
};


//  These are thrown for failed system errors. They are not necessarily bugs.
class InternalSystemError : public Exception{
public:
    InternalSystemError(Logger* logger, const char* location, std::string message);
    virtual const char* name() const override{ return "InternalSystemError"; }
    virtual std::string message() const override;
private:
    const char* m_location;
    std::string m_message;
};


class UserSetupError : public Exception{
public:
    UserSetupError(Logger& logger, std::string message);
    virtual const char* name() const override{ return "UserSetupError"; }
    virtual std::string message() const override;
private:
    std::string m_message;
};







}
#endif
