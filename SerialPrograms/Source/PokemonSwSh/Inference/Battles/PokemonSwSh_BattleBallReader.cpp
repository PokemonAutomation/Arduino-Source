/*  Ball Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/StringNormalization.h"
#include "CommonFramework/OCR/Filtering.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ImageMatch::ExactImageMatcher make_BALL_SPRITE_MATCHER(){
    ImageMatch::ExactImageMatcher matcher;
    for (const auto& item : all_pokeball_sprites()){
        matcher.add(item.first, item.second.sprite());
    }
    return matcher;
}
const ImageMatch::ExactImageMatcher& BALL_SPRITE_MATCHER(){
    static ImageMatch::ExactImageMatcher matcher = make_BALL_SPRITE_MATCHER();
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
    , m_box_sprite(console, 0.649, 0.624, 0.0335, 0.060)
    , m_box_name(console, 0.710, 0.624, 0.18, 0.060)
    , m_box_quantity(console, 0.895, 0.624, 0.059, 0.060)
{}

std::string BattleBallReader::read_ball(const QImage& screen) const{
    if (screen.isNull()){
        return "";
    }

    ImageMatch::MatchResult sprite_result;
    {
#if 0
        QImage cropped = extract_box(screen, m_box_sprite);
//        cropped.save("cropped.png");
        m_matcher.scale_to_dimensions(cropped);
//        cropped.save("scaled.png");
        sprite_result = m_matcher.match(cropped, false, 15);
#else
        sprite_result = m_matcher.match(screen, m_box_sprite, false, 2, 0.02);
#endif
        sprite_result.log(m_console, 0.30);
        if (!sprite_result.slugs.empty() && sprite_result.slugs.begin()->first > 0.30){
            sprite_result.slugs.clear();
        }
    }
    OCR::MatchResult name_result;
    {
        QImage cropped = extract_box(screen, m_box_name);
        OCR::make_OCR_filter(cropped).apply(cropped);
        name_result = m_name_reader.read_substring(m_language, cropped);
        name_result.log(m_console);
    }

    if (sprite_result.slugs.empty()){
        if (name_result.slugs.size() == 1){
            return *name_result.slugs.begin();
        }
        dump_image(m_console, screen, "BattleBallReader");
        return "";
    }

    if (sprite_result.slugs.size() == 1){
        return sprite_result.slugs.begin()->second;
    }

    std::vector<std::string> overlap;
    for (const auto& item : sprite_result.slugs){
        auto iter = name_result.slugs.find(item.second);
        if (iter != name_result.slugs.end()){
            overlap.emplace_back(item.second);
        }
    }
    if (overlap.size() != 1){
        dump_image(m_console, screen, "BattleBallReader");
        return "";
    }
    return overlap[0];
}
int BattleBallReader::read_quantity(const QImage& screen) const{
    QImage image = extract_box(screen, m_box_quantity);
    auto filter = OCR::make_OCR_filter(image);
    filter.apply(image);
    QString ocr_text = OCR::ocr_read(Language::English, image);
    QString normalized;
    for (QChar ch : ocr_text){
        if (ch.isDigit()){
            normalized += ch;
        }
    }
    int quantity = normalized.toInt();

    QString str;
    for (QChar ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    m_console.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> " + QString::number(quantity));
    return quantity;
}






}
}
}
