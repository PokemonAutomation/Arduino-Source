/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OperationFailedException_H
#define PokemonAutomation_OperationFailedException_H

#include <memory>
#include "ScreenshotException.h"

namespace PokemonAutomation{

class FatalProgramException;


//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OperationFailedException : public ScreenshotException{
public:
    using ScreenshotException::ScreenshotException;
    explicit OperationFailedException(ErrorReport error_report, Logger& logger, std::string message);
    explicit OperationFailedException(ErrorReport error_report, Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot);
    explicit OperationFailedException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot);

    virtual const char* name() const override{ return "OperationFailedException"; }

};





}
#endif
