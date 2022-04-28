/*  In-Battle Ball Inventory Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonBDSP/Inference/PokemonBDSP_PokeballSpriteMatcher.h"
#include "PokemonBDSP_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const double BattleBallReader::MAX_ALPHA = 0.40;
const double BattleBallReader::ALPHA_SPREAD = 0.02;


const PokeballSpriteMatcher& BALL_SPRITE_MATCHER(){
    static PokeballSpriteMatcher matcher;
    return matcher;
}



BattleBallReader::BattleBallReader(
    ConsoleHandle& console,
    Language language
)
    : m_matcher(BALL_SPRITE_MATCHER())
    , m_name_reader(PokeballNameReader::instance())
    , m_language(language)
    , m_console(console)
    , m_box_sprite(console, 0.617, 0.650, 0.0335, 0.060)
    , m_box_name(console, 0.650, 0.650, 0.22, 0.060)
    , m_box_quantity(console, 0.880, 0.650, 0.070, 0.060)
{}



std::string BattleBallReader::read_ball(const QImage& screen) const{
    if (screen.isNull()){
        return "";
    }

    ImageMatch::ImageMatchResult sprite_result;
    {
        ConstImageRef image = extract_box_reference(screen, m_box_sprite);
        sprite_result = m_matcher.match(image, ALPHA_SPREAD);
        sprite_result.log(m_console, 0.50);
        if (!sprite_result.results.empty() && sprite_result.results.begin()->first > MAX_ALPHA){
            sprite_result.results.clear();
        }
    }
    OCR::StringMatchResult name_result;
    {
        ConstImageRef cropped = extract_box_reference(screen, m_box_name);
        name_result = m_name_reader.read_substring(
            m_console, m_language, cropped,
            {
                {0xff000000, 0xff404040},
                {0xff000000, 0xff808080},
            }
        );
    }

    if (sprite_result.results.empty()){
        if (name_result.results.size() == 1){
            return name_result.results.begin()->second.token;
        }
        dump_image(m_console, ProgramInfo(), "BattleBallReader", screen);
        return "";
    }

    if (sprite_result.results.size() == 1){
        return sprite_result.results.begin()->second;
    }

    std::set<std::string> ocr_slugs;
    for (const auto& item : name_result.results){
        ocr_slugs.insert(item.second.token);
    }

    std::vector<std::string> overlap;
    for (const auto& item : sprite_result.results){
        auto iter = ocr_slugs.find(item.second);
        if (iter != ocr_slugs.end()){
            overlap.emplace_back(item.second);
        }
    }
    if (overlap.size() != 1){
        dump_image(m_console, ProgramInfo(), "BattleBallReader", screen);
        return "";
    }
    return overlap[0];
}

uint16_t BattleBallReader::read_quantity(const QImage& screen) const{
    QImage image = extract_box_copy(screen, m_box_quantity);
    to_blackwhite_rgb32_range(image, 0xff808080, 0xffffffff, true);
    return OCR::read_number(m_console, image);
}



}
}
}
