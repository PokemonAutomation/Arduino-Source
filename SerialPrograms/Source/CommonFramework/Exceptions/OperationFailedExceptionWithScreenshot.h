/*  Operation Failed Exception with Screenshot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_OperationFailedExceptionWithScreenshot_H
#define PokemonAutomation_OperationFailedExceptionWithScreenshot_H

#include <memory>
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
class EventNotificationOption;
class VideoStream;
class ProgramEnvironment;


//  Thrown by subroutines if they fail for an in-game reason.
//  These include recoverable errors which can be consumed by the program.
class OperationFailedExceptionWithScreenshot : public OperationFailedException{
public:
    OperationFailedExceptionWithScreenshot(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream
    );

    //  Construct exception with message with screenshot and (optionally) console information.
    //  Use the provided screenshot instead of taking one with the console.
    //  Store the console information (if provided) for stream history if requested later.
    OperationFailedExceptionWithScreenshot(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        ImageRGB32 screenshot
    );
    OperationFailedExceptionWithScreenshot(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        std::shared_ptr<const ImageRGB32> screenshot
    );

    //  This is the most common use case. Throw and log exception.
    //  Include console information for screenshot and stream history.
    [[noreturn]] static void fire(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream
    );
    [[noreturn]] static void fire(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream,
        std::shared_ptr<const ImageRGB32> screenshot
    );

    virtual const char* name() const override{ return "OperationFailedExceptionWithScreenshot"; }
    ImageViewRGB32 screenshot_view() const;
    std::shared_ptr<const ImageRGB32> screenshot() const;
    VideoStream* video_stream() const;

    void send_recoverable_error_notif_and_telemetry_report(
        ProgramEnvironment& env,
        EventNotificationOption& notif_settings
    ) override;

    void send_fatal_error_notif_and_telemetry_report(
        ProgramEnvironment& env,
        EventNotificationOption& notif_settings
    ) override;

private:
    VideoStream* m_stream = nullptr;
    std::shared_ptr<const ImageRGB32> m_screenshot;
};





}
#endif
