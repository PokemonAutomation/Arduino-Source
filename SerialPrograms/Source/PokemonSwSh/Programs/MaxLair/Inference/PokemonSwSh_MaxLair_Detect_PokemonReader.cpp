/*  Max Lair Pokemon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/OCR/Filtering.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::set<std::string> read_pokemon_name(
    Logger& logger,
    const QImage& screen,
    const ImageFloatBox& box,
    Language language
){
    QImage image = extract_box(screen, box);
    OCR::TextImageFilter filter = OCR::make_OCR_filter(image);
    filter.apply(image);

    OCR::MatchResult result = Pokemon::PokemonNameReader::instance().read_substring(language, image);
    result.log(logger);
    if (!result.matched){
        dump_image(logger, screen, "MaxLair-NameOCR");
        return {};
    }

    //  TODO: Use DA list to eliminate impossible matches.

    return result.slugs;
}


std::string read_pokemon_sprite(Logger& logger, const QImage& image){
    double max_RMSD = 150;
    ImageMatch::MatchResult result = read_pokemon_sprite_on_solid(logger, image, max_RMSD);
    auto iter = result.slugs.begin();
    if (iter == result.slugs.end()){
        return "";
    }
    if (iter->first > max_RMSD){
        return "";
    }
    if (iter->first > 100 && result.slugs.size() > 5){
        return "";
    }
    return std::move(iter->second);
}


std::string read_pokemon_name_sprite(
    Logger& logger,
    const QImage& screen,
    const ImageFloatBox& sprite_box,
    const ImageFloatBox& name_box,
    Language language
){
    QImage sprite = extract_box(screen, sprite_box);
    QImage name = extract_box(screen, name_box);

    std::set<std::string> ocr_slugs = read_pokemon_name(logger, screen, name_box, language);
    bool ocr_hit = !ocr_slugs.empty();
    bool ocr_unique = ocr_hit && ocr_slugs.size() == 1;

    double max_RMSD = 150;
    ImageMatch::MatchResult result = read_pokemon_sprite_on_solid(logger, extract_box(screen, sprite_box), max_RMSD);
    auto iter = result.slugs.begin();
    bool sprite_hit = iter != result.slugs.end() && iter->first <= max_RMSD;
    if (!sprite_hit || result.slugs.size() > 1){
        dump_image(logger, screen, "MaxLair-read_name_sprite");
    }

    //  No hit on sprite. Use OCR.
    if (!sprite_hit){
        return ocr_unique ? *ocr_slugs.begin() : "";
    }

    //  No hit on OCR. Use sprite.
    if (!ocr_hit){
        return sprite_hit ? std::move(iter->second) : "";
    }

    //  If any are in common to both, pick the best one.
    std::multimap<double, std::string> common;
    for (const auto& item : result.slugs){
        if (ocr_slugs.find(item.second) != ocr_slugs.end()){
            common.emplace(item.first, item.second);
        }
    }
    if (!common.empty()){
        return std::move(common.begin()->second);
    }

    //  This is where things get bad since OCR and sprites disagree.

    //  If there is only one sprite match, use it.
    if (result.slugs.size() == 1){
        return std::move(result.slugs.begin()->second);
    }

    //  If there is only one OCR match, use it.
    if (ocr_slugs.size() == 1){
        return *ocr_slugs.begin();
    }

    //  At this point, both OCR and sprites have multiple items in completely disjoint sets.
    return "";
}

std::string read_raid_mon(
    Logger& logger,
    VideoOverlay& overlay,
    const QImage& screen,
    Language language
){
    InferenceBoxScope box(overlay, 0.3, 0.010, 0.4, 0.10, Qt::green);
    std::set<std::string> slugs = read_pokemon_name(logger, screen, box, language);
    if (slugs.empty() || slugs.size() != 1){
        return "";
    }
    return *slugs.begin();
}




}
}
}
}
