/*  Debug Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_DebugDumper_H
#define PokemonAutomation_DebugDumper_H

#include <string>

namespace PokemonAutomation{

class ImageViewRGB32;
class Logger;

//  Dump debug image to `DEBUG_PATH()`/`path`/<timestamp>-`label`.png
//  Return image path.
//  See SerialPrograms/Source/CommonFramework/GlobalAutoPaths.h for `DEBUG_PATH()`.
std::string dump_debug_image(
    Logger& logger,
    const std::string& path,
    const std::string& label,
    const ImageViewRGB32& image
);

}
#endif
