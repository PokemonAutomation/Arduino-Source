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
    explicit ScreenshotException(ErrorReport error_report, std::string message);
    explicit ScreenshotException(ErrorReport error_report, std::string message, std::shared_ptr<const ImageRGB32> screenshot);
    explicit ScreenshotException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot);
    explicit ScreenshotException(
        ErrorReport error_report,
        std::string message,
        ConsoleHandle& console
    );
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
