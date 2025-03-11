/*  Program Finished Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramFinishedException_H
#define PokemonAutomation_ProgramFinishedException_H

#include <memory>
#include "ScreenshotException.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
class EventNotificationOption;
struct ProgramInfo;
class ProgramEnvironment;



//  Thrown when the program requests a normal stop to the program.
//    - This should not be consumed except by the infra.
//    - Non-infra are allowed to catch and rethrow this exception.
class ProgramFinishedException : public ScreenshotException{
public:
    ProgramFinishedException();
    explicit ProgramFinishedException(std::string message);

    //  Construct exception with message and console information.
    //  This will take a screenshot and store the console if the stream history if requested later.
    explicit ProgramFinishedException(
        std::string message,
        VideoStream& stream
    );

    //  Construct exception with message with screenshot and (optionally) console information.
    //  Use the provided screenshot instead of taking one with the console.
    //  Store the console information (if provided) for stream history if requested later.
    explicit ProgramFinishedException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        ImageRGB32 screenshot
    );
    explicit ProgramFinishedException(
        std::string message,
        VideoStream* stream,
        std::shared_ptr<const ImageRGB32> screenshot
    );

    virtual Color color() const override{ return COLOR_GREEN; }

public:
    virtual void log(Logger& logger) const override;
    virtual const char* name() const override{ return "ProgramFinishedException"; }
};





}
#endif
