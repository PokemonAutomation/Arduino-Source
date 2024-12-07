/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OperationFailedException_H
#define PokemonAutomation_OperationFailedException_H

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "ScreenshotException.h"

namespace PokemonAutomation{


//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OperationFailedException : public ScreenshotException{
public:
    using ScreenshotException::ScreenshotException;

    //  This is the most common use case. Throw and log exception.
    //  Include console information for screenshot and stream history.
    [[noreturn]] static void fire(
        ConsoleHandle& console,
        ErrorReport error_report,
        std::string message
    ){
        throw_and_log<OperationFailedException>(console, error_report, std::move(message), console);
    }
    [[noreturn]] static void fire(
        ConsoleHandle& console,
        ErrorReport error_report,
        std::string message,
        std::shared_ptr<const ImageRGB32> screenshot
    ){
        throw_and_log<OperationFailedException>(console, error_report, std::move(message), &console, std::move(screenshot));
    }

    virtual const char* name() const override{ return "OperationFailedException"; }
};





}
#endif
