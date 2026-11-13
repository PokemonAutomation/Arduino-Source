/*  Pokemon FRLG Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PokemonSpriteReader_H
#define PokemonAutomation_PokemonFRLG_PokemonSpriteReader_H

#include <memory>
#include <set>
#include <vector>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/ImageMatchResult.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"

namespace PokemonAutomation{

class UnitTestDatabase;

namespace NintendoSwitch{
namespace PokemonFRLG{


//  Matches front sprites at every position in a rectangle on the game screen.
//  Positions are the sprite's top-left corner, in GBA pixels (240 x 160).
//  Transparent sprite pixels in the reference sheet are ignored, so the background doesn't matter.
class PositionedSpriteMatcher{
public:
    struct Result{
        ImageMatch::ImageMatchResult species;
        bool shiny = false;
    };

public:
    PositionedSpriteMatcher(
        const std::set<std::string>* subset,
        bool flipped, const std::vector<bool>& shiny_variants,
        int x_min, int x_max, int y_min, int y_max
    );

    ImageFloatBox search_box() const;

    Result match(const ImageViewRGB32& game_screen, double alpha_spread) const;

private:
    struct Template{
        std::string species;
        bool shiny;
        ImageMatch::WeightedExactImageMatcher matcher;
    };

    int m_x_min;
    int m_x_max;
    int m_y_min;
    int m_y_max;
    std::vector<std::unique_ptr<Template>> m_templates;
};


// Reads the sprite on the summary screen (any page).
// The sprite bounces when the summary is opened, so a range of vertical positions is searched.
class SummarySpriteReader{
public:
    SummarySpriteReader(const std::set<std::string>& subset, Color color = COLOR_RED, bool shiny = false);

    void make_overlays(VideoOverlaySet& items) const;

    ImageMatch::ImageMatchResult read(const ImageViewRGB32& frame) const;

private:
    Color m_color;
    PositionedSpriteMatcher m_matcher;
};


// Reads the opponent's sprite on the battle screen.
// The sprite's vertical position depends on the species (and whether it floats)
class BattleSpriteReader{
public:
    using Result = PositionedSpriteMatcher::Result;

public:
    BattleSpriteReader(const std::set<std::string>* subset = nullptr, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    Result read(const ImageViewRGB32& frame) const;

private:
    Color m_color;
    PositionedSpriteMatcher m_matcher;
};


void add_tests_SummarySpriteReader(UnitTestDatabase& database);
void add_tests_BattleSpriteReader(UnitTestDatabase& database);


}
}
}
#endif
