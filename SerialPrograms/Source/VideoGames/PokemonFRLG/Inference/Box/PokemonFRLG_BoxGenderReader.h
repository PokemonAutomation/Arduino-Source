/*  Box Gender Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxGenderReader_H
#define PokemonAutomation_PokemonFRLG_BoxGenderReader_H

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class BoxGenderReader{
public:
    BoxGenderReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    // Genderless also covers an empty selection with no gender symbol.
    Pokemon::StatsHuntGenderFilter read(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_box;
};

void add_tests_BoxGenderReader(UnitTestDatabase& database);

}
}
}
#endif
