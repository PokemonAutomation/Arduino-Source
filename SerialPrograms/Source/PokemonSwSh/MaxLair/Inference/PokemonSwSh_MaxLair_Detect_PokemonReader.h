/*  Max Lair Pokemon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonReader_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PokemonReader_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::string read_boss_sprite(ConsoleHandle& console);


//  OCR the Pokemon name and return all possible candidates.
std::set<std::string> read_pokemon_name(
    Logger& logger, Language language,
    const ImageViewRGB32& image,
    double max_log10p = -1.4
);


//  Read a Pokemon sprite. The sprite may be partially obstructed
//  on the right side by an item.
std::string read_pokemon_sprite_with_item(
    Logger& logger,
    const ImageViewRGB32& screen, const ImageFloatBox& box
);



//  Use both OCR and sprite to read the Pokemon. It will try to
//  arbitrate conflicts.
std::string read_pokemon_name_sprite(
    Logger& logger,
    const ImageViewRGB32& screen,
    const ImageFloatBox& sprite_box,
    const ImageFloatBox& name_box, Language language,
    bool allow_exact_match_fallback
);



}
}
}
}
#endif
