/*  Ball Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const double BattleBallReader::MAX_ALPHA = 0.29;
const double BattleBallReader::ALPHA_SPREAD = 0.02;


ImageMatch::ExactImageDictionaryMatcher make_BALL_SPRITE_MATCHER(){
    ImageMatch::ExactImageDictionaryMatcher matcher({1, 128});
    for (const auto& item : ALL_POKEBALL_SPRITES()){
        matcher.add(item.first, item.second.sprite.copy());
    }
    return matcher;
}
const ImageMatch::ExactImageDictionaryMatcher& BALL_SPRITE_MATCHER(){
    static ImageMatch::ExactImageDictionaryMatcher matcher = make_BALL_SPRITE_MATCHER();
    return matcher;
}



BattleBallReader::BattleBallReader(
    VideoStream& stream,
    Language language
)
    : m_matcher(BALL_SPRITE_MATCHER())
    , m_name_reader(PokeballNameReader::instance())
    , m_language(language)
    , m_stream(stream)
    , m_box_sprite(stream.overlay(), {0.649, 0.624, 0.0335, 0.060})
    , m_box_name(stream.overlay(), {0.710, 0.624, 0.18, 0.060})
    , m_box_quantity(stream.overlay(), {0.895, 0.624, 0.059, 0.060})
{}

std::string BattleBallReader::read_ball(const ImageViewRGB32& screen) const{
    if (!screen){
        return "";
    }

    ImageMatch::ImageMatchResult sprite_result;
    {
        sprite_result = m_matcher.match(screen, m_box_sprite, 2, ALPHA_SPREAD);
        sprite_result.log(m_stream.logger(), 0.50);
        if (!sprite_result.results.empty() && sprite_result.results.begin()->first > MAX_ALPHA){
            sprite_result.results.clear();
        }
    }
    if (sprite_result.results.empty()){
        ImageViewRGB32 sprite = extract_box_reference(screen, m_box_sprite);
        ImageStats stats = image_stats(sprite);
        if (stats.stddev.sum() > 20){
            dump_image(m_stream.logger(), ProgramInfo(), "BattleBallReader-Sprite", screen);
        }
    }

    OCR::StringMatchResult name_result;
    {
        ImageViewRGB32 cropped = extract_box_reference(screen, m_box_name);
        name_result = m_name_reader.read_substring(
            m_stream.logger(), m_language, cropped,
            OCR::WHITE_TEXT_FILTERS()
        );
    }
    if (name_result.results.size() != 1){
        dump_image(m_stream.logger(), ProgramInfo(), "BattleBallReader-Name", screen);
    }

    if (sprite_result.results.empty()){
        if (name_result.results.size() == 1){
            return name_result.results.begin()->second.token;
        }
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
        dump_image(m_stream.logger(), ProgramInfo(), "BattleBallReader-SpriteNameMismatch", screen);
        return "";
    }
    return overlap[0];
}
uint16_t BattleBallReader::read_quantity(const ImageViewRGB32& screen) const{
    ImageRGB32 image = to_blackwhite_rgb32_range(
        extract_box_reference(screen, m_box_quantity),
        true,
        0xff808080, 0xffffffff
    );
    int qty = OCR::read_number(m_stream.logger(), image);
    return (uint16_t)std::max(qty, 0);
}






}
}
}
