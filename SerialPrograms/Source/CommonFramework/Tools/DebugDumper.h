/*  Debug Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_DebugDumper_H
#define PokemonAutomation_DebugDumper_H


namespace PokemonAutomation{

class ImageViewRGB32;
class LoggerQt;

// Dump debug image to ./DebugDumps/`path`/<timestamp>-`label`.jpg
// Return image path.
std::string dump_debug_image(
    LoggerQt& logger,
    const std::string& path,
    const std::string& label,
    const ImageViewRGB32& image
);

}
#endif
