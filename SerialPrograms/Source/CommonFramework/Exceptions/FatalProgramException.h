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

namespace PokemonAutomation{


//  A generic exception that should not be caught outside of infra.
class FatalProgramException : public Exception{
public:

    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message
    )
        : m_error_report_mode(error_report)
        , m_message(message)
    {}

    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message,
        VideoStream& stream
    )
        : m_error_report_mode(error_report)
        , m_message(message)
        , m_stream(&stream)
        , m_screenshot(stream.video().snapshot().frame)
    {}

    //  Construct exception with message with screenshot and (optionally) console information.
    //  Use the provided screenshot instead of taking one with the console.
    //  Store the console information (if provided) for stream history if requested later.
    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        ImageRGB32 screenshot
    )
        : m_error_report_mode(error_report)
        , m_message(message)
        , m_stream(stream)
        , m_screenshot(std::make_shared<ImageRGB32>(std::move(screenshot)))
    {}

    explicit FatalProgramException(
        ErrorReport error_report,
        std::string message,
        VideoStream* stream,
        std::shared_ptr<const ImageRGB32> screenshot
    )
        : m_error_report_mode(error_report)
        , m_message(message)
        , m_stream(stream)
        , m_screenshot(std::move(screenshot))
    {}

    ErrorReport error_report_mode() const { return m_error_report_mode; };
    virtual const char* name() const override{ return "FatalProgramException"; }
    ImageViewRGB32 screenshot_view() const {
        if (m_screenshot){
            return *m_screenshot;
        }else{
            return ImageViewRGB32();
        }
    }
    std::shared_ptr<const ImageRGB32> screenshot() const {return m_screenshot;}
    VideoStream* video_stream() const{return m_stream;};

private:
    ErrorReport m_error_report_mode;
    std::string m_message;
    VideoStream* m_stream = nullptr;
    std::shared_ptr<const ImageRGB32> m_screenshot;
};





}
#endif
