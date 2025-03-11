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

// Dump error image to ./ErrorDumps/ folder. Also send image as telemetry if user allows.
void dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image,
    const StreamHistorySession* stream_history = nullptr
);
void dump_image(
    Logger& logger,
    const ProgramInfo& program_info,
    VideoFeed& video,
    const std::string& label
);

// dump a screenshot to ./ErrorDumps/ folder and throw an OperationFailedException.
// error_name: the error name, used as the image name and show up on video overlay log. Typical format example:
//   "NoHatchEnd", "NoYCommFound".
// error_message: the exception mesage.
[[noreturn]] void dump_image_and_throw_recoverable_exception(
    const ProgramInfo& program_info,
    VideoStream& stream,
    const std::string& error_name,
    const std::string& error_message,
    const ImageViewRGB32& screenshot = ImageViewRGB32()
);


}
#endif
