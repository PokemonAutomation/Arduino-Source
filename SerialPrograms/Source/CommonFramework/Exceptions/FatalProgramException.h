/*  Fatal Program Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_FatalProgramException_H
#define PokemonAutomation_FatalProgramException_H

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"

namespace PokemonAutomation{


//  A generic exception that should not be caught outside of infra.
class FatalProgramException : public Exception{
public:

    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message
    );

    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream
    );

    //  Construct exception with message with screenshot and (optionally) console information.
    //  Use the provided screenshot instead of taking one with the console.
    //  Store the console information (if provided) for stream history if requested later.
    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        ImageRGB32 screenshot
    );

    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        std::shared_ptr<const ImageRGB32> screenshot
    );

    ErrorReport error_report_mode() const { return m_error_report_mode; }
    virtual const char* name() const override{ return "FatalProgramException"; }
    ImageViewRGB32 screenshot_view() const {
        if (m_screenshot){
            return *m_screenshot;
        }else{
            return ImageViewRGB32();
        }
    }
    std::shared_ptr<const ImageRGB32> screenshot() const {return m_screenshot;}
    VideoStream* video_stream() const{return m_stream;}

    void send_fatal_error_notif_and_telemetry_report(
        ProgramEnvironment& env,
        EventNotificationOption& notif_settings
    );

private:
    ErrorReport m_error_report_mode;
    std::string m_message;
    VideoStream* m_stream = nullptr;
    std::shared_ptr<const ImageRGB32> m_screenshot;
};





}
#endif
