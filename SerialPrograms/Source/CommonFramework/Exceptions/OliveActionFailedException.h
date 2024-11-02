/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OliveActionFailedException_H
#define PokemonAutomation_OliveActionFailedException_H

#include <memory>
#include "OperationFailedException.h"

namespace PokemonAutomation{

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
    explicit OliveActionFailedException(ErrorReport error_report, Logger& logger, std::string message, OliveFail fail_reason = OliveFail::NONE);
    explicit OliveActionFailedException(ErrorReport error_report, Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot, OliveFail fail_reason = OliveFail::NONE);
    explicit OliveActionFailedException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot, OliveFail fail_reason = OliveFail::NONE);

    virtual const char* name() const override{ return "OliveActionFailedException"; }

public:
    OliveFail m_fail_reason;

};





}
#endif
