/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/
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
    OperationFailedException(
        ErrorReport error_report_mode,
        std::string message
    )
        : m_error_report_mode(error_report_mode)
        , m_message(std::move(message))
    {}

    ErrorReport error_report_mode() const { return m_error_report_mode; };

    virtual const char* name() const override{ return "OperationFailedException"; }
    virtual std::string message() const override{ return m_message; }

private:
    ErrorReport m_error_report_mode;
    std::string m_message;
};





}
#endif
