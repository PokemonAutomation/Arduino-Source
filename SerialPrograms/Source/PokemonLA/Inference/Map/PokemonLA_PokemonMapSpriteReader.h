/*  Pokemon Map Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H
#define PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H

#include <vector>
#include <map>
#include "CommonTools/ImageMatch/ExactImageDictionaryMatcher.h"
#include "PokemonLA/PokemonLA_Locations.h"

struct ImagePixelBox;

namespace PokemonAutomation{


namespace NintendoSwitch{
namespace PokemonLA{

struct MapSpriteMatchResult{
    std::string slug;

    std::vector<std::string> candidates;

    std::multimap<double, std::string> color_match_results;

    std::multimap<double, std::string> gradient_match_results;

    double color_score = 0.0;
    
    double color_lead = 0.0;

    double gradient_score = 0.0;
    
    double gradient_lead = 0.0;

    double graident_top_color_score = 0.0;
    
    double gradient_second_color_score = 0.0;
    
    bool pick_gradient_second = false;

    std::string second_slug;
};

MapSpriteMatchResult match_sprite_on_map(
    Logger& logger,
    const ImageViewRGB32& screen,
    const ImagePixelBox& box,
    MapRegion region,
    bool debug_mode = false
);


}
}
}
#endif
