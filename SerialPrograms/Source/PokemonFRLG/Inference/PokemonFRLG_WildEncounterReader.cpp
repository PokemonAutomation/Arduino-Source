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
#include "PokemonFRLG_OcrPreprocessing.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


WildEncounterReader::WildEncounterReader(Color color)
    : m_box_name(0.075, 0.120, 0.260, 0.063) 
    , m_box_name_jpn(0.075, 0.120, 0.232, 0.063)
    // , m_box_level(0.325, 0.120, 0.092, 0.063)
    {}

void WildEncounterReader::make_overlays(VideoOverlaySet& items) const {
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_name));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_name_jpn));
    // items.add(m_color, GAME_BOX.inner_to_outer(m_box_level));
}

PokemonFRLG_WildEncounter WildEncounterReader::read_encounter(
    Logger& logger, Language language,
    const ImageViewRGB32& frame, 
    const std::set<std::string>& subset,
    double max_log10p
){
    const bool jpn = language == Language::Japanese;

    PokemonFRLG_WildEncounter encounter; 
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 name_box = extract_box_reference(game_screen, jpn ? m_box_name_jpn : m_box_name);

    //  Dark text on the light battle HP bar. Blur to close the gaps in the GBA
    //  font, then let the matcher try each threshold in turn.
    ImageRGB32 name_ready = preprocess_for_ocr(name_box);

    auto name_result = Pokemon::PokemonNameReader(subset).read_substring(
            logger, language, name_ready,
            DARK_TEXT_FILTERS(),
            0.01, 0.50, max_log10p);
    if (!name_result.results.empty()){
        encounter.name = name_result.results.begin()->second.token;
    }else{
    logger.log("Failed to read species name.", COLOR_RED);
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            name_box.save("DebugDumps/ocr_encounter_box.png");
            name_ready.save("DebugDumps/ocr_encounter_ready.png");
            game_screen.save("DebugDumps/ocr_encounter_frame.png");
        }
    }
    return encounter;
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

