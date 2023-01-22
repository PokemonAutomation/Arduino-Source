/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OperationFailedException_H
#define PokemonAutomation_OperationFailedException_H

#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{


//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OperationFailedException : public Exception{
public:
//    OperationFailedException(std::string message) : m_message(message) {}
    OperationFailedException(Logger& logger, std::string message);
    virtual const char* name() const override{ return "OperationFailedException"; }
    virtual std::string message() const override{ return m_message; }
private:
    std::string m_message;
};



}
#endif
