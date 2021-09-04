/*  Pokemon Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokemonSpriteReader_H
#define PokemonAutomation_PokemonSwSh_PokemonSpriteReader_H

#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/ImageMatch/ImageMatchResult.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ImageMatch::MatchResult read_pokemon_sprite_on_solid(
    Logger& logger,
    const QImage& image,
    double max_RMSD = 150
);




}
}
}
#endif
