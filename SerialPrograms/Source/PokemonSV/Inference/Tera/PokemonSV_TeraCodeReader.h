/*  Tera Code Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCodeReader_H
#define PokemonAutomation_PokemonSV_TeraCodeReader_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{


void preload_code_templates();


//  Returns # of seconds left. Returns -1 if unable to read.
int16_t read_raid_timer(Logger& logger, const ImageViewRGB32& image);

//  Returns empty string if unable to read.
std::string read_raid_code(Logger& logger, const ImageViewRGB32& image);



}
}
}
#endif
