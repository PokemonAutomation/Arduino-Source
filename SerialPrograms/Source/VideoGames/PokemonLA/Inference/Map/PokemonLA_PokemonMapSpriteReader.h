/*  Pokemon Map Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H
#define PokemonAutomation_PokemonLA_PokemonMapSpriteReader_H

#include <vector>
#include <map>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonTools/ImageMatch/ExactImageDictionaryMatcher.h"
#include "VideoGames/PokemonLA/PokemonLA_Locations.h"

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

// Read which pokemon sprite is at `box` on the region map `screen` of `region`:
// 1. Pick the 10 sprites whose features are closest.
// 2. Match their colors. Stop if the best color match leads the second by enough.
// 3. Otherwise match their gradients, and fall back on color when gradients are close.
// It logs each step and the result at `LogLevel::Verbose`, since a map has many sprites.
// `debug_mode`: also save the sprite image with `dump_debug_image()`, in a folder named
//               after the matched slug.
MapSpriteMatchResult match_sprite_on_map(
    Logger& logger,
    const ImageViewRGB32& screen,
    const ImagePixelBox& box,
    MapRegion region,
    bool debug_mode = false
);




void add_tests_PokemonMapSpriteReader(UnitTestDatabase& database);




}
}
}
#endif
