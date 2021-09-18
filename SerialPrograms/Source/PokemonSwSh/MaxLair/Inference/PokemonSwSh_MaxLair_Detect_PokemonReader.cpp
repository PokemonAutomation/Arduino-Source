/*  Max Lair Pokemon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/OCR/Filtering.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


struct SpeciesReadDatabase{
    static constexpr double MAX_SPRITE_RMSD = 0.40;

    std::map<std::string, std::set<std::string>> ocr_map;
    std::map<std::string, std::set<std::string>> sprite_map;

    std::unique_ptr<PokemonNameReader> name_reader;
    std::unique_ptr<PokemonSpriteMatcherCropped> sprite_reader;
    std::unique_ptr<PokemonSpriteMatcherExact> exact_sprite_reader;
    std::unique_ptr<PokemonLeftSpriteMatcherExact> exact_leftsprite_reader;

    static SpeciesReadDatabase& instance(){
        static SpeciesReadDatabase data;
        return data;
    }

    SpeciesReadDatabase(){
        QJsonObject json = read_json_file(
            PERSISTENT_SETTINGS().resource_path + QString::fromStdString("PokemonSwSh/MaxLairSlugMap.json")
        ).object();

        std::set<std::string> ocr_set;
        std::set<std::string> sprite_set;

        for (auto iter = json.begin(); iter != json.end(); ++iter){
            std::string maxlair_slug = iter.key().toStdString();
            QJsonObject obj = iter.value().toObject();
            for (const auto& item : obj["OCR"].toArray()){
                std::string slug = item.toString().toStdString();
                ocr_map[slug].insert(maxlair_slug);
                ocr_set.insert(slug);
            }
            for (const auto& item : obj["Sprite"].toArray()){
                std::string slug = item.toString().toStdString();
                sprite_map[slug].insert(maxlair_slug);
                sprite_set.insert(slug);
            }
        }

        name_reader.reset(new PokemonNameReader(ocr_set));
        sprite_reader.reset(new PokemonSpriteMatcherCropped(&sprite_set));
        exact_sprite_reader.reset(new PokemonSpriteMatcherExact(&sprite_set));
        exact_leftsprite_reader.reset(new PokemonLeftSpriteMatcherExact(&sprite_set));
    }
};


std::set<std::string> read_pokemon_name(
    Logger& logger,
    const QImage& screen, const QImage& image,
    Language language
){
    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();

//    OCR::MatchResult result = PokemonNameReader::instance().read_substring(language, image);
    OCR::MatchResult result = database.name_reader->read_substring(language, image);
    result.log(logger);
    if (!result.matched){
        dump_image(logger, screen, "MaxLair-NameOCR");
        return {};
    }

    //  Convert OCR slugs to MaxLair name slugs.
    std::set<std::string> ret;
    for (const std::string& name_slug : result.slugs){
        auto iter = database.ocr_map.find(name_slug);
        if (iter == database.ocr_map.end()){
            PA_THROW_StringException("Slug doesn't exist in OCR map: " + name_slug);
        }
        for (const std::string& item : iter->second){
            ret.insert(item);
        }
    }

    return ret;
}


ImageMatch::MatchResult read_pokemon_sprite_set(
    Logger& logger,
    const QImage& screen,
    const ImageFloatBox& box,
    bool allow_exact_match_fallback
){
//    const QImage& sprite = get_pokemon_sprite("galvantula").sprite();
//    sprite.save("sprite.png");
//    extract_box(screen, box).scaled(sprite.size()).save("test.png");


    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();

    //  Try with cropped matcher.
    ImageFloatBox large_box = box;
    large_box.height += 0.002;
//    large_box.width -= 0.010;
//    large_box.x += 0.005;
    QImage image = extract_box(screen, large_box);

//    image.save("test.png");

    ImageMatch::MatchResult result = database.sprite_reader->match(image);
//    result.log(logger, SpeciesReadDatabase::MAX_SPRITE_RMSD);

    //  Try with exact matcher.
    if (allow_exact_match_fallback){
        if (result.slugs.size() != 1 || result.slugs.begin()->first > 0.30){
            logger.log("Retrying with exact sprite matcher...");
            result = database.exact_sprite_reader->match(screen, box, false, 5);
        }
    }

    //  Convert slugs to MaxLair slugs.
    for (auto& item : result.slugs){
        auto iter = database.sprite_map.find(item.second);
        if (iter == database.ocr_map.end()){
            PA_THROW_StringException("Slug doesn't exist in sprite map: " + item.second);
        }
        if (iter->second.empty()){
            PA_THROW_StringException("Sprite map is empty for slug: " + item.second);
        }
        item.second = *iter->second.begin();
    }

    result.log(logger, SpeciesReadDatabase::MAX_SPRITE_RMSD);

    return result;
}
#if 0
std::string read_pokemon_sprite(
    Logger& logger,
    const QImage& screen, const ImageFloatBox& box
){
    ImageMatch::MatchResult result = read_pokemon_sprite_set(logger, screen, box);
    auto iter = result.slugs.begin();
    if (iter == result.slugs.end()){
        return "";
    }
    if (iter->first > SpeciesReadDatabase::MAX_SPRITE_RMSD){
        logger.log("No good sprite match found.", Qt::red);
        return "";
    }
//    if (iter->first > 0.35 && result.slugs.size() > 5){
//        logger.log("Multiple \"ok\" sprite matches with no clear winner.", Qt::red);
//        return "";
//    }
    return std::move(iter->second);
}
#endif




ImageMatch::MatchResult read_pokemon_sprite_set_with_item(Logger& logger, const QImage& screen, const ImageFloatBox& box){
    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();

    //  Try with cropped matcher.
    ImageFloatBox large_box = box;
    large_box.height += 0.002;
    large_box.width -= 0.010;
    large_box.x += 0.005;
    QImage image = extract_box(screen, large_box);

    ImageMatch::MatchResult result = database.sprite_reader->match(image);

    //  Try with exact matcher.
    if (result.slugs.empty() || result.slugs.begin()->first > SpeciesReadDatabase::MAX_SPRITE_RMSD){
        logger.log("Retrying with left-side exact sprite matcher...");
        ImageFloatBox half_box = box;
        half_box.width /= 2;
        result = database.exact_leftsprite_reader->match(screen, half_box, false, 5);
    }

    //  Convert slugs to MaxLair slugs.
    for (auto& item : result.slugs){
        auto iter = database.sprite_map.find(item.second);
        if (iter == database.ocr_map.end()){
            PA_THROW_StringException("Slug doesn't exist in sprite map: " + item.second);
        }
        if (iter->second.empty()){
            PA_THROW_StringException("Sprite map is empty for slug: " + item.second);
        }
        item.second = *iter->second.begin();
    }

    result.log(logger, SpeciesReadDatabase::MAX_SPRITE_RMSD);

    return result;
}
std::string read_pokemon_sprite_with_item(
    Logger& logger,
    const QImage& screen, const ImageFloatBox& box
){
    ImageMatch::MatchResult result = read_pokemon_sprite_set_with_item(logger, screen, box);
    auto iter = result.slugs.begin();
    if (iter == result.slugs.end()){
        return "";
    }
    if (iter->first > SpeciesReadDatabase::MAX_SPRITE_RMSD){
        logger.log("No good sprite match found.", Qt::red);
        return "";
    }
    if (iter->first > 100 && result.slugs.size() > 5){
        logger.log("Multiple \"ok\" sprite matches with no clear winner.", Qt::red);
        return "";
    }
    return std::move(iter->second);
}


std::string read_pokemon_name_sprite(
    Logger& logger,
    const QImage& screen,
    const ImageFloatBox& sprite_box,
    const ImageFloatBox& name_box, Language language,
    bool allow_exact_match_fallback
){
//    QImage sprite = extract_box(screen, sprite_box);
    QImage name = extract_box(screen, name_box);

    QImage image = extract_box(screen, name_box);
    OCR::filter_smart(image);

    std::set<std::string> ocr_slugs = read_pokemon_name(logger, screen, image, language);
    bool ocr_hit = !ocr_slugs.empty();
    bool ocr_unique = ocr_hit && ocr_slugs.size() == 1;

    ImageMatch::MatchResult result = read_pokemon_sprite_set(
        logger,
        screen,
        sprite_box,
        allow_exact_match_fallback
    );
    auto iter = result.slugs.begin();
    bool sprite_hit = iter != result.slugs.end() && iter->first <= SpeciesReadDatabase::MAX_SPRITE_RMSD;
    if (!sprite_hit || result.slugs.size() > 1){
        dump_image(logger, screen, "MaxLair-read_name_sprite");
    }

    //  No hit on sprite. Use OCR.
    if (!sprite_hit){
        std::string ret = ocr_unique ? *ocr_slugs.begin() : "";
        logger.log("Failed to read sprite. Using OCR result: " + ret, Qt::red);
        dump_image(logger, screen, "MaxLair-read_name_sprite");
        return ret;
    }

    //  No hit on OCR. Use sprite.
    if (!ocr_hit){
        std::string ret = sprite_hit ? std::move(iter->second) : "";
        logger.log("Failed to read name. Using sprite result: " + ret, Qt::red);
        dump_image(logger, screen, "MaxLair-read_name_sprite");
        return ret;
    }

    //  If any are in common to both, pick the best one.
    std::multimap<double, std::string> common;
    for (const auto& item : result.slugs){
        if (ocr_slugs.find(item.second) != ocr_slugs.end()){
            common.emplace(item.first, item.second);
        }
    }
    if (!common.empty()){
        logger.log("Sprite and OCR agree. Picking best one: " + common.begin()->second, Qt::blue);
        return std::move(common.begin()->second);
    }

    //  This is where things get bad since OCR and sprites disagree.
    dump_image(logger, screen, "MaxLair-read_name_sprite");

    //  If there is only one sprite match, use it.
    if (result.slugs.size() == 1){
        logger.log("Sprite and OCR disagree. Attempt to arbitrate... Picking: " + result.slugs.begin()->second, Qt::red);
        return std::move(result.slugs.begin()->second);
    }

    //  If there is only one OCR match, use it.
    if (ocr_slugs.size() == 1){
        logger.log("Sprite and OCR disagree. Attempt to arbitrate... Picking: " + *ocr_slugs.begin(), Qt::red);
        return *ocr_slugs.begin();
    }

    //  At this point, both OCR and sprites have multiple items in completely disjoint sets.
    logger.log("Sprite and OCR disagree so badly that no arbitration will be attempted.", Qt::red);
    return "";
}





}
}
}
}
