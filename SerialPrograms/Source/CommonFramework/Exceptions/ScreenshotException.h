/*  Screenshot Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ScreenshotException_H
#define PokemonAutomation_ScreenshotException_H

#include <memory>
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
class EventNotificationOption;
class VideoStream;
struct ProgramInfo;
class ProgramEnvironment;


enum class ErrorReport{
    NO_ERROR_REPORT,
    SEND_ERROR_REPORT,
};


//  Base class for program exception holding a screenshot.
//
//  Do not use this class directly. It is just to reuse the screenshot holding
//  logic that's shared by multiple exception types.
class ScreenshotException : public Exception{
public:
    ScreenshotException() = default;

    //  Construct exception with a simple message.
    explicit ScreenshotException(ErrorReport error_report, std::string message);

    //  Construct exception with message and console information.
    //  This will take a screenshot and store the console if the stream history if requested later.
    explicit ScreenshotException(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream
    );

    //  Construct exception with message with screenshot and (optionally) console information.
    //  Use the provided screenshot instead of taking one with the console.
    //  Store the console information (if provided) for stream history if requested later.
    explicit ScreenshotException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        ImageRGB32 screenshot
    );
    explicit ScreenshotException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        std::shared_ptr<const ImageRGB32> screenshot
    );

    //  Add console information if it isn't already requested.
    //  This will provide screenshot and stream history if requested later.
    void add_stream_if_needed(VideoStream& stream);


public:
//    virtual const char* name() const override{ return "ScreenshotException"; }
    virtual std::string message() const override{ return m_message; }
    ImageViewRGB32 screenshot_view() const;
    std::shared_ptr<const ImageRGB32> screenshot() const;

    virtual Color color() const{ return COLOR_RED; }
    virtual void send_notification(ProgramEnvironment& env, EventNotificationOption& notification, const std::string& title_prefix = "") const;
    void send_recoverable_notification(ProgramEnvironment& env) const;
    void send_fatal_notification(ProgramEnvironment& env) const;

public:
    ErrorReport m_send_error_report;
    std::string m_message;
    VideoStream* m_stream = nullptr;
    std::shared_ptr<const ImageRGB32> m_screenshot;
};





}
#endif
