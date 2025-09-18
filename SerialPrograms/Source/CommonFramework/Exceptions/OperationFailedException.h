/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OperationFailedException_H
#define PokemonAutomation_OperationFailedException_H

#include <memory>
#include "CommonFramework/Tools/VideoStream.h"
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
        ErrorReport error_report,
        std::string message,
        VideoStream& stream
    ){
        throw_and_log<OperationFailedException>(stream.logger(), error_report, std::move(message), stream);
    }
    [[noreturn]] static void fire(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream,
        std::shared_ptr<const ImageRGB32> screenshot
    ){
        throw_and_log<OperationFailedException>(stream.logger(), error_report, std::move(message), &stream, std::move(screenshot));
    }

    virtual const char* name() const override{ return "OperationFailedException"; }
};





}
#endif
