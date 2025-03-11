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

// Dump debug image to ./DebugDumps/`path`/<timestamp>-`label`.png
// Return image path.
std::string dump_debug_image(
    Logger& logger,
    const std::string& path,
    const std::string& label,
    const ImageViewRGB32& image
);

}
#endif
