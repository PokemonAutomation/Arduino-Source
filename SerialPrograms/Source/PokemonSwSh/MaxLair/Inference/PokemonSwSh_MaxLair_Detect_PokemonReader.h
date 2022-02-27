/*  Max Lair Pokemon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonReader_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonReader_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::string read_boss_sprite(ConsoleHandle& console);


//  OCR the Pokemon name and return all possible candidates.
std::set<std::string> read_pokemon_name(
    LoggerQt& logger,
    const QImage& screen, const QImage& image,
    Language language
);


#if 0
//  Read a Pokemon sprite. This will attempt to auto-sizing first.
//  If that fails, it will attempt exact match.
std::string read_pokemon_sprite(
    Logger& logger,
    const QImage& screen, const ImageFloatBox& box
);
#endif


//  Read a Pokemon sprite. The sprite may be partially obstructed
//  on the right side by an item.
std::string read_pokemon_sprite_with_item(
    LoggerQt& logger,
    const QImage& screen, const ImageFloatBox& box
);



//  Use both OCR and sprite to read the Pokemon. It will try to
//  arbitrate conflicts.
std::string read_pokemon_name_sprite(
    LoggerQt& logger,
    const QImage& screen,
    const ImageFloatBox& sprite_box,
    const ImageFloatBox& name_box, Language language,
    bool allow_exact_match_fallback
);



}
}
}
}
#endif
