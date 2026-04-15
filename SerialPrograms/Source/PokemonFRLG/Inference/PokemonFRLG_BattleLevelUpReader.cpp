/*  Battle Level Up Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_BattleLevelUpReader.h"
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
#include "Pokemon/Inference/Pokemon_NatureReader.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DigitReader.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


BattleLevelUpReader::BattleLevelUpReader(Color color)
    : m_color(color)
    , m_box_hp(0.904069, 0.402852, 0.068535, 0.079387)
    , m_box_attack(0.904069, 0.496052, 0.068535, 0.079387)
    , m_box_defense(0.904069, 0.589252, 0.068535, 0.079387)
    , m_box_sp_attack(0.904069, 0.682452, 0.068535, 0.079387)
    , m_box_sp_defense(0.904069, 0.775652, 0.068535, 0.079387)
    , m_box_speed(0.904069, 0.868852, 0.068535, 0.0793879)
{}

void BattleLevelUpReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_hp));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_attack));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_defense));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sp_attack));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sp_defense));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_speed));
}

PokemonFRLG_LevelUpStats BattleLevelUpReader::read_stats(Logger &logger, const ImageViewRGB32& frame){
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    
    auto read_stat = [&](const ImageFloatBox &box, const std::string &name){
        ImageViewRGB32 stat_region = extract_box_reference(game_screen, box);

        if (!GlobalSettings::instance().USE_PADDLE_OCR){
            // Tesseract-free path: waterfill segmentation + template matching
            // against the PokemonFRLG/Digits/0-9.png templates.
            return read_digits_waterfill_template(logger, stat_region);
        }

        // PaddleOCR path (original): preprocess then per-digit waterfill OCR.
        // Dark text [0..190] -> black. Threshold at 190 captures the
        // blurred gap pixels between segments, making bridges thicker.
        // Not higher than 190 to avoid capturing yellow bg edge noise.
        ImageRGB32 ocr_ready = preprocess_for_ocr(
            stat_region, name, 7, 2, true,
            combine_rgb(0, 0, 0), combine_rgb(190, 190, 190)
        );

        // Waterfill isolates each digit -> per-char SINGLE_CHAR OCR.
        return OCR::read_number_waterfill(
            logger, ocr_ready, 0xff000000,
            0xff808080
        );
    };

    PokemonFRLG_LevelUpStats stats;
    auto assign_stat = [](std::optional<unsigned>& field, int value){
        if (value != -1){
            field = static_cast<unsigned>(value);
        }
    };
    assign_stat(stats.hp, read_stat(m_box_hp, "hp"));
    assign_stat(stats.attack, read_stat(m_box_attack, "attack"));
    assign_stat(stats.defense, read_stat(m_box_defense, "defense"));
    assign_stat(stats.sp_attack, read_stat(m_box_sp_attack, "spatk"));
    assign_stat(stats.sp_defense, read_stat(m_box_sp_defense, "spdef"));
    assign_stat(stats.speed, read_stat(m_box_speed, "speed"));
    return stats;
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

