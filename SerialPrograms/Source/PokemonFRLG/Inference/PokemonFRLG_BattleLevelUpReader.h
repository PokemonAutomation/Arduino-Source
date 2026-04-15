/*  Battle Level Up Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleLevelUpReader_H
#define PokemonAutomation_PokemonFRLG_BattleLevelUpReader_H

#include <optional>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"


namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonFRLG{

struct PokemonFRLG_LevelUpStats{
    std::optional<unsigned> hp;
    std::optional<unsigned> attack;
    std::optional<unsigned> defense;
    std::optional<unsigned> sp_attack;
    std::optional<unsigned> sp_defense;
    std::optional<unsigned> speed;
};

class BattleLevelUpReader {
public:
    BattleLevelUpReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet &items) const;

    PokemonFRLG_LevelUpStats read_stats(Logger &logger, const ImageViewRGB32& frame);

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

