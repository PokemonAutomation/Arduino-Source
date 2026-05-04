/*  Party Level Up Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PartyLevelUpReader_H
#define PokemonAutomation_PokemonFRLG_PartyLevelUpReader_H

#include <optional>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "Pokemon/Pokemon_StatsCalculation.h"


namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace Pokemon;


class PartyLevelUpReader {
public:
    PartyLevelUpReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet &items) const;

    StatReads read_stats(Logger &logger, const ImageViewRGB32& frame) const;

private:
    Color m_color;
    ImageFloatBox m_box_hp;
    ImageFloatBox m_box_attack;
    ImageFloatBox m_box_defense;
    ImageFloatBox m_box_sp_attack;
    ImageFloatBox m_box_sp_defense;
    ImageFloatBox m_box_speed;

};

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
#endif

