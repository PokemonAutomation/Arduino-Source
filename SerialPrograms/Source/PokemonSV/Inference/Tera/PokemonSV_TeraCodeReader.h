/*  Tera Code Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCodeReader_H
#define PokemonAutomation_PokemonSV_TeraCodeReader_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{



std::string read_raid_code(Logger& logger, const ImageViewRGB32& image);



}
}
}
#endif
