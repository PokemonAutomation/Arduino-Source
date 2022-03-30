/*  Max Lair Detect Path Side
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSide_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSide_H

#include <stdint.h>
#include "CommonFramework/ImageTypes/ImageReference.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



int8_t read_side(const ConstImageRef& image);


}
}
}
}
#endif
