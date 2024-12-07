/*  Screenshot Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
struct ProgramInfo;
class ProgramEnvironment;
class ConsoleHandle;


enum class ErrorReport{
    NO_ERROR_REPORT,
    SEND_ERROR_REPORT,
};


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
        ConsoleHandle& console
    );

    //  Construct exception with message with screenshot and (optionally) console information.
    //  Use the provided screenshot instead of taking one with the console.
    //  Store the console information (if provided) for stream history if requested later.
    explicit ScreenshotException(
        ErrorReport error_report,
        std::string message,
        ConsoleHandle* console,
        ImageRGB32 screenshot
    );
    explicit ScreenshotException(
        ErrorReport error_report,
        std::string message,
        ConsoleHandle* console,
        std::shared_ptr<const ImageRGB32> screenshot
    );

    //  Add console information if it isn't already requested.
    //  This will provide screenshot and stream history if requested later.
    void add_console_if_needed(ConsoleHandle& console);


public:
//    virtual const char* name() const override{ return "ScreenshotException"; }
    virtual std::string message() const override{ return m_message; }
    ImageViewRGB32 screenshot() const;

    virtual void send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const;

public:
    ErrorReport m_send_error_report;
    std::string m_message;
    ConsoleHandle* m_console = nullptr;
    std::shared_ptr<const ImageRGB32> m_screenshot;
};





}
#endif
