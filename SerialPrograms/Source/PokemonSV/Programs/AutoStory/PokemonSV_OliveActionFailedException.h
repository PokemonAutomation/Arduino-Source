/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OliveActionFailedException_H
#define PokemonAutomation_OliveActionFailedException_H

#include "CommonFramework/Exceptions/OperationFailedException.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class FatalProgramException;

enum class OliveFail{
    NONE,
    FAILED_ALIGN_TO_OLIVE,
    FAILED_PUSH_OLIVE_TOTAL_DISTANCE,
    NO_OLIVE_DETECTED,
    FAILED_WALK_TO_OLIVE,
    OLIVE_STUCK,
};

//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OliveActionFailedException : public OperationFailedException{
public:
    OliveActionFailedException(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream,
        OliveFail fail_reason = OliveFail::NONE
    )
        : OperationFailedException(error_report, std::move(message), stream)
        , m_fail_reason(fail_reason)
    {}
    virtual const char* name() const override{ return "OliveActionFailedException"; }

public:
    OliveFail m_fail_reason;

};




}
}
}
#endif
