/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ErrorDumper_H
#define PokemonAutomation_ErrorDumper_H

#include "CommonFramework/Notifications/ProgramNotifications.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class Logger;

// Dump error image to ./ErrorDumps/ folder.
// Return image path.
std::string dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image
);


}
#endif
