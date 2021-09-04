/*  Max Lair Detect PP
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PP_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PP_H

#include <stdint.h>
#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


int8_t read_pp_from_swap_menu(Logger& logger, QImage image);


}
}
}
}
#endif
