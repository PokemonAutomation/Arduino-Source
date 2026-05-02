/*  Stats Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StatsReader_H
#define PokemonAutomation_PokemonFRLG_StatsReader_H

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


enum class SummaryGender{
    Male,
    Female,
    Genderless
};

struct PokemonFRLG_Stats{
    std::string nature;
    std::optional<unsigned> level;
    std::optional<SummaryGender> gender;
    std::string name;
    std::optional<unsigned> hp;
    std::optional<unsigned> attack;
    std::optional<unsigned> defense;
    std::optional<unsigned> sp_attack;
    std::optional<unsigned> sp_defense;
    std::optional<unsigned> speed;
};

class StatsReader {
public:
    StatsReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet &items) const;

    // Reads from page 1 (Nature, Level, Name)
    void read_page1(
        Logger &logger, Language language,
        const ImageViewRGB32 &frame, PokemonFRLG_Stats &stats,
        const std::set<std::string>& subset = {}
    );

    // Reads from page 2 (Stats: HP, Atk, Def, SpA, SpD, Spe)
    void read_page2(
        Logger &logger, const ImageViewRGB32 &frame,
        PokemonFRLG_Stats &stats
    );

private:
    Color m_color;
    ImageFloatBox m_box_nature;
    ImageFloatBox m_box_level;
    ImageFloatBox m_box_name;
    ImageFloatBox m_box_gender;
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

