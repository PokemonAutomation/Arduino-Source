/*  Party Level Up Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon/Pokemon_StatsCalculation.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DigitReader.h"
#include "PokemonFRLG_PartyLevelUpReader.h"


namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


PartyLevelUpReader::PartyLevelUpReader(Color color)
    : m_color(color)
    , m_box_hp(0.844069, 0.051852, 0.128535, 0.079387)
    , m_box_attack(0.844069, 0.145052, 0.128535, 0.079387)
    , m_box_defense(0.844069, 0.238252, 0.128535, 0.079387)
    , m_box_sp_attack(0.844069, 0.331452, 0.128535, 0.079387)
    , m_box_sp_defense(0.844069, 0.424652, 0.128535, 0.079387)
    , m_box_speed(0.844069, 0.517852, 0.128535, 0.0793879)
{}

void PartyLevelUpReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_hp));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_attack));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_defense));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sp_attack));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sp_defense));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_speed));
}

StatReads PartyLevelUpReader::read_stats(Logger &logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    
    auto read_stat = [&](const ImageFloatBox &box, const std::string &name){
        ImageViewRGB32 stat_region = extract_box_reference(game_screen, box);
        return read_digits_waterfill_template(logger, stat_region);
    };

    StatReads stats;
    stats.hp = uint16_t(read_stat(m_box_hp, "hp"));
    stats.attack = uint16_t(read_stat(m_box_attack, "attack"));
    stats.defense = uint16_t(read_stat(m_box_defense, "defense"));
    stats.spatk = uint16_t(read_stat(m_box_sp_attack, "spatk"));
    stats.spdef = uint16_t(read_stat(m_box_sp_defense, "spdef"));
    stats.speed = uint16_t(read_stat(m_box_speed, "speed"));
    return stats;
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

