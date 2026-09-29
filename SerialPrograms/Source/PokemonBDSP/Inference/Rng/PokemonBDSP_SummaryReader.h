/*  BDSP Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SummaryReader_H
#define PokemonAutomation_PokemonBDSP_SummaryReader_H

#include <stdint.h>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "Pokemon/Pokemon_BdspRng.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Pokemon_StatsCalculation.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonBDSP{


const LanguageSet& summary_nature_languages();


class SummaryReader{
public:
    SummaryReader(Color color = COLOR_RED);

    void make_memo_overlays(VideoOverlaySet& items) const;
    void make_skills_overlays(VideoOverlaySet& items) const;

    Pokemon::NatureCheckerValue read_nature(
        Logger& logger, Language language, const ImageViewRGB32& frame
    ) const;

    Pokemon::BdspGender read_gender(Logger& logger, const ImageViewRGB32& frame) const;

    Pokemon::StatReads read_stats(Logger& logger, const ImageViewRGB32& frame) const;

private:
    int16_t read_stat(
        Logger& logger, const ImageViewRGB32& frame, const ImageFloatBox& box
    ) const;

    int16_t read_hp_total(Logger& logger, const ImageViewRGB32& frame) const;

    Color m_color;
    ImageFloatBox m_box_nature;
    ImageFloatBox m_box_gender;
    ImageFloatBox m_box_hp;
    ImageFloatBox m_box_attack;
    ImageFloatBox m_box_defense;
    ImageFloatBox m_box_spatk;
    ImageFloatBox m_box_spdef;
    ImageFloatBox m_box_speed;
};


}
}
}
#endif
