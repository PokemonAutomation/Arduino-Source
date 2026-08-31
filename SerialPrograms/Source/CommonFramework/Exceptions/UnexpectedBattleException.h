/*  Unexpected Battle Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UnexpectedBattleException_H
#define PokemonAutomation_UnexpectedBattleException_H

#include "OperationFailedExceptionWithScreenshot.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

//  Thrown by subroutines if caught in an wild battle in-game unexpectedly.
//  These include recoverable errors which can be consumed by the program.
class UnexpectedBattleException : public OperationFailedExceptionWithScreenshot{
public:
    UnexpectedBattleException(
        ErrorReportMode error_report_mode,
        std::string message,
        VideoStream& stream
    )
        : OperationFailedExceptionWithScreenshot(error_report_mode, std::move(message), stream)
    {}

    virtual const char* name() const override{ return "UnexpectedBattleException"; }
};





}
}
#endif
