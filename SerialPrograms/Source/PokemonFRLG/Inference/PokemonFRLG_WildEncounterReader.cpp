/*  Wild Encounter Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_WildEncounterReader.h"
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
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DigitReader.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


WildEncounterReader::WildEncounterReader(Color color)
    : m_box_name(0.075, 0.120, 0.265, 0.063) 
    // , m_box_level(0.325, 0.120, 0.092, 0.063)
    {}

void WildEncounterReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_name));
    // items.add(m_color, GAME_BOX.inner_to_outer(m_box_level));
}

void WildEncounterReader::read_encounter(
    Logger &logger, Language language,
    const ImageViewRGB32 &frame, 
    std::set<std::string> &subset, 
    PokemonFRLG_WildEncounter& encounter
) {
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    // Read Name (black text on off-white background).
    // Use multifiltered OCR across multiple black bands. This tolerates
    // brightness shifts (down to ~0xc0) while still preferring cleaner bands.
    const std::vector<OCR::TextColorRange> name_text_color_ranges{
        {combine_rgb(0, 0, 0), combine_rgb(64, 64, 64)},
        {combine_rgb(0, 0, 0), combine_rgb(96, 96, 96)},
        {combine_rgb(0, 0, 0), combine_rgb(128, 128, 128)},
    };
    // auto name_result = Pokemon::PokemonNameReader::instance().read_substring(    
    auto name_result = Pokemon::PokemonNameReader(subset).read_substring(
            logger, language, extract_box_reference(game_screen, m_box_name),
            name_text_color_ranges);
    if (!name_result.results.empty()) {
        encounter.name = name_result.results.begin()->second.token;
    }
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

