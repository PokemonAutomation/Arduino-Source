/*  BDSP Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxNatureDetector.h"
#include "PokemonBDSP_SummaryReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


static const std::vector<OCR::TextColorRange>& MEMO_TEXT_FILTERS(){
    static std::vector<OCR::TextColorRange> filters{
        //  Red.
        {0xff600000, 0xffff8080},
        {0xff800000, 0xffff6060},
        //  Black, as a fallback.
        {0xff000000, 0xff606060},
        {0xff000000, 0xff909090},
    };
    return filters;
}


const LanguageSet& summary_nature_languages(){
    return NATURE_READER().languages();
}


SummaryReader::SummaryReader(Color color)
    : m_color(color)
    , m_box_nature (0.055, 0.190, 0.410, 0.062)
    , m_box_gender (0.790, 0.094, 0.024, 0.040)
    , m_box_hp     (0.228, 0.168, 0.080, 0.054)
    , m_box_attack (0.376, 0.288, 0.048, 0.052)
    , m_box_defense(0.376, 0.436, 0.048, 0.052)
    , m_box_spatk  (0.112, 0.288, 0.048, 0.052)
    , m_box_spdef  (0.112, 0.436, 0.048, 0.052)
    , m_box_speed  (0.242, 0.543, 0.048, 0.052)
{}

void SummaryReader::make_memo_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_nature, "nature");
    items.add(m_color, m_box_gender, "gender");
}

void SummaryReader::make_skills_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_gender, "gender");
    items.add(m_color, m_box_hp, "hp");
    items.add(m_color, m_box_attack, "atk");
    items.add(m_color, m_box_defense, "def");
    items.add(m_color, m_box_spatk, "spatk");
    items.add(m_color, m_box_spdef, "spdef");
    items.add(m_color, m_box_speed, "spe");
}


NatureCheckerValue SummaryReader::read_nature(
    Logger& logger, Language language, const ImageViewRGB32& frame
) const{
    if (language == Language::None){
        return NatureCheckerValue::UnableToDetect;
    }
    ImageViewRGB32 line = extract_box_reference(frame, m_box_nature);
    OCR::StringMatchResult result = NATURE_READER().read_substring(
        logger, language, line, MEMO_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(NatureReader::MAX_LOG10P);
    if (result.results.size() != 1){
        return NatureCheckerValue::UnableToDetect;
    }
    return NATURE_CHECKER_VALUE_STRINGS().get_enum(result.results.begin()->second.token);
}


BdspGender SummaryReader::read_gender(Logger& logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 box = extract_box_reference(frame, m_box_gender);

    size_t blue_pixels = 0;
    size_t pink_pixels = 0;
    for (size_t y = 0; y < box.height(); y++){
        for (size_t x = 0; x < box.width(); x++){
            uint32_t pixel = box.pixel(x, y);
            uint32_t red   = (pixel >> 16) & 0xff;
            uint32_t green = (pixel >> 8) & 0xff;
            uint32_t blue  = pixel & 0xff;
            if (blue > red + 40 && blue > green + 20){
                blue_pixels++;
            }else if (red > green + 40 && blue > green + 40){
                pink_pixels++;
            }
        }
    }
    logger.log("Gender symbol: " + std::to_string(blue_pixels) + " blue pixels, "
        + std::to_string(pink_pixels) + " pink.");
    if (blue_pixels == 0 && pink_pixels == 0){
        return BdspGender::Genderless;
    }
    return blue_pixels > pink_pixels ? BdspGender::Male : BdspGender::Female;
}


int16_t SummaryReader::read_stat(
    Logger& logger, const ImageViewRGB32& frame, const ImageFloatBox& box
) const{
    ImageViewRGB32 image = extract_box_reference(frame, box);
    int value = OCR::read_number_waterfill(logger, image, 0xff000000, 0xff808080);
    if (value < 5 || value > 40){
        return -1;
    }
    return (int16_t)value;
}


int16_t SummaryReader::read_hp_total(Logger& logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 image = extract_box_reference(frame, m_box_hp);
    int value = OCR::read_number_waterfill(logger, image, 0xff000000, 0xff808080);
    if (value < 0){
        return -1;
    }
    std::string digits = std::to_string(value);
    if (digits.size() < 2){
        //  Only the current HP came through, or nothing did. Either way the total is
        //  not in there.
        return -1;
    }
    int total = std::stoi(digits.substr(digits.size() - 2));
    logger.log("HP read as \"" + digits + "\", taking " + std::to_string(total)
        + " as the total.");
    if (total < 15 || total > 30){
        return -1;
    }
    return (int16_t)total;
}


StatReads SummaryReader::read_stats(Logger& logger, const ImageViewRGB32& frame) const{
    StatReads ret;
    ret.hp      = read_hp_total(logger, frame);
    ret.attack  = read_stat(logger, frame, m_box_attack);
    ret.defense = read_stat(logger, frame, m_box_defense);
    ret.spatk   = read_stat(logger, frame, m_box_spatk);
    ret.spdef   = read_stat(logger, frame, m_box_spdef);
    ret.speed   = read_stat(logger, frame, m_box_speed);
    return ret;
}


}
}
}
