/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ErrorDumper_H
#define PokemonAutomation_ErrorDumper_H

#include <string>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{

class VideoFeed;
class StreamHistorySession;
class VideoStream;
class EventNotificationOption;
class ImageViewRGB32;
class Logger;
struct VideoSnapshot;
class ProgramEnvironment;
struct ProgramInfo;

#if 0
std::string dump_image_alone(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image
);
#endif

// Call CommonFramework/ErrorReports/ErrorReports.h:report_error() to create an error
// report with image. Check the comments of report_error() for more details.
void dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image,
    const StreamHistorySession* stream_history = nullptr
);

// Take a screenshot from the video stream and call
// CommonFramework/ErrorReports/ErrorReports.h:report_error() to create an error
// report with the screenshot. Check the comments of report_error() for more details.
void dump_image(
    Logger& logger,
    const ProgramInfo& program_info,
    VideoFeed& video,
    const std::string& label
);

// Throw an OperationFailedException that will trigger error report creation.
// Check OperationFailedException::fire() for more details.
[[noreturn]] void dump_image_and_throw_recoverable_exception(
    const ProgramInfo& program_info,
    VideoStream& stream,
    const std::string& error_name,
    const std::string& error_message,
    const ImageViewRGB32& screenshot = ImageViewRGB32()
);


}
#endif
