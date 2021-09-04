/*  Max Lair Pokemon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonReader_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonReader_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::string read_pokemon_sprite(Logger& logger, const QImage& image);

std::string read_pokemon_name_sprite(
    Logger& logger,
    const QImage& screen,
    const ImageFloatBox& sprite_box,
    const ImageFloatBox& name_box,
    Language language
);

std::string read_raid_mon(
    Logger& logger,
    VideoOverlay& overlay,
    const QImage& screen,
    Language language
);


}
}
}
}
#endif
