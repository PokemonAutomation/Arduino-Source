/*  Max Lair Detect HP and PP
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_HPPP_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_HPPP_H

#include <stdint.h>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


//double read_hp_bar(Logger& logger, const ImageViewRGB32& image);
Health read_in_battle_hp_box(Logger& logger, const ImageViewRGB32& sprite, const ImageViewRGB32& hp_bar);
int8_t read_pp_text(Logger& logger, const ImageViewRGB32& image);


}
}
}
}
#endif
