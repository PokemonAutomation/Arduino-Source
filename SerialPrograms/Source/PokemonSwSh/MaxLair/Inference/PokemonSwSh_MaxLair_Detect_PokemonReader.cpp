/*  Max Lair Pokemon Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


struct SpeciesReadDatabase{

    static constexpr double CROPPED_MAX_ALPHA = 0.30;
    static constexpr double CROPPED_ALPHA_SPREAD = 0.03;

    static constexpr double RETRY_ALPHA = 0.25;

    static constexpr double EXACT_MAX_ALPHA = 0.30;
    static constexpr double EXACT_ALPHA_SPREAD = 0.02;


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
        std::set<std::string> ocr_set;
        std::set<std::string> sprite_set;

        for (const auto& item0 : maxlair_slugs()){
            const std::string& maxlair_slug = item0.first;
            {
                const std::string& slug = item0.second.name_slug;
                ocr_map[slug].insert(maxlair_slug);
                ocr_set.insert(slug);
            }
            for (const auto& slug : item0.second.sprite_slugs){
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



std::string read_boss_sprite(VideoStream& stream){
    DenMonReader reader(stream.logger(), stream.overlay());
    DenMonReadResults results = reader.read(stream.video().snapshot());

    std::string sprite_slug;
    {
        auto iter = results.slugs.results.begin();
        if (iter == results.slugs.results.end()){
            return "";
        }
        sprite_slug = std::move(iter->second);
    }

    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();
    auto iter = database.sprite_map.find(sprite_slug);
    if (iter == database.sprite_map.end()){
        throw InternalProgramError(
            &stream.logger(), PA_CURRENT_FUNCTION,
            "Slug doesn't exist in sprite map: " + sprite_slug
        );
    }
    if (iter->second.empty()){
        throw InternalProgramError(
            &stream.logger(), PA_CURRENT_FUNCTION,
            "Sprite map is empty for slug: " + sprite_slug
        );
    }
    return *iter->second.begin();
}


std::set<std::string> read_pokemon_name(
    Logger& logger, Language language,
    OcrFailureWatchdog& ocr_watchdog,
    const ImageViewRGB32& image,
    double max_log10p
){
    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();

//    OCR::MatchResult result = PokemonNameReader::instance().read_substring(language, image);
//    image.save("test.png");
    OCR::StringMatchResult result = database.name_reader->read_substring(
        logger, language, image,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS(),
        0.01, 0.50,
        max_log10p
    );
//    result.log(logger);
    if (result.results.empty()){
        ocr_watchdog.push_result(false);
        return {};
    }
    ocr_watchdog.push_result(true);

    //  Convert OCR slugs to MaxLair name slugs.
    std::set<std::string> ret;
    for (const auto& hit : result.results){
        auto iter = database.ocr_map.find(hit.second.token);
        if (iter == database.ocr_map.end()){
            throw InternalProgramError(
                &logger, PA_CURRENT_FUNCTION,
                "Slug doesn't exist in OCR map: " + hit.second.token
            );
        }
        for (const std::string& item : iter->second){
            ret.insert(item);
        }
    }

    return ret;
}


ImageMatch::ImageMatchResult read_pokemon_sprite_set(
    Logger& logger,
    const ImageViewRGB32& screen,
    const ImageFloatBox& box,
    bool allow_exact_match_fallback
){
    using namespace papkmnlib;

    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();
    const std::map<std::string, Pokemon>& RENTALS = all_rental_pokemon();

    //  Try with cropped matcher.
    ImageFloatBox large_box = box;
    large_box.height += 0.002;
//    large_box.width -= 0.010;
//    large_box.x += 0.005;
    ImageViewRGB32 image = extract_box_reference(screen, large_box);

//    image.save("test.png");

    double max_alpha = SpeciesReadDatabase::CROPPED_MAX_ALPHA;

    ImageMatch::ImageMatchResult result = database.sprite_reader->match(image, SpeciesReadDatabase::CROPPED_ALPHA_SPREAD);
//    result.log(logger, SpeciesReadDatabase::CROPPED_MAX_ALPHA);

    //  Try with exact matcher.
    if (allow_exact_match_fallback){
        if (result.results.size() != 1 || result.results.begin()->first > SpeciesReadDatabase::RETRY_ALPHA){
            logger.log("Retrying with exact sprite matcher...");
            max_alpha = SpeciesReadDatabase::EXACT_MAX_ALPHA;
            result = database.exact_sprite_reader->match(
                screen, box,
                5,
                SpeciesReadDatabase::EXACT_ALPHA_SPREAD
            );
        }
    }

    //  Convert slugs to MaxLair slugs.
    std::multimap<double, std::string> filtered;
    for (auto& item : result.results){
        auto iter = database.sprite_map.find(item.second);
        if (iter == database.sprite_map.end()){
            throw InternalProgramError(
                &logger, PA_CURRENT_FUNCTION,
                "Slug doesn't exist in sprite map: " + item.second
            );
        }
        if (iter->second.empty()){
            throw InternalProgramError(
                &logger, PA_CURRENT_FUNCTION,
                "Sprite map is empty for slug: " + item.second
            );
        }
//        item.second = *iter->second.begin();

        //  Include it only if it's a valid rental Pokemon.
        const std::string& slug = *iter->second.begin();
        if (RENTALS.find(slug) != RENTALS.end()){
            filtered.emplace(item.first, slug);
        }
    }
    result.results = std::move(filtered);

    result.log(logger, max_alpha);
    result.clear_beyond_alpha(max_alpha);

    return result;
}




ImageMatch::ImageMatchResult read_pokemon_sprite_set_with_item(
    Logger& logger,
    const ImageViewRGB32& screen, const ImageFloatBox& box
){
    const SpeciesReadDatabase& database = SpeciesReadDatabase::instance();

    //  Try with cropped matcher.
    ImageFloatBox large_box = box;
    large_box.height += 0.002;
    large_box.width -= 0.010;
    large_box.x += 0.005;
    ImageViewRGB32 image = extract_box_reference(screen, large_box);

    double max_alpha = SpeciesReadDatabase::CROPPED_MAX_ALPHA;

    ImageMatch::ImageMatchResult result = database.sprite_reader->match(image, SpeciesReadDatabase::CROPPED_ALPHA_SPREAD);
//    result.results.clear();
//    result.log(logger, SpeciesReadDatabase::CROPPED_MAX_ALPHA);

    //  Try with exact matcher.
    if (result.results.empty() || result.results.begin()->first > SpeciesReadDatabase::RETRY_ALPHA){
        logger.log("Retrying with left-side exact sprite matcher...");
        max_alpha = SpeciesReadDatabase::EXACT_MAX_ALPHA;
        ImageFloatBox half_box = box;
        half_box.width /= 2;
        result = database.exact_leftsprite_reader->match(screen, half_box, 5, SpeciesReadDatabase::EXACT_ALPHA_SPREAD);
    }

    //  Convert slugs to MaxLair slugs.
    for (auto& item : result.results){
        auto iter = database.sprite_map.find(item.second);
        if (iter == database.sprite_map.end()){
            throw InternalProgramError(
                &logger, PA_CURRENT_FUNCTION,
                "Slug doesn't exist in sprite map: " + item.second
            );
        }
        if (iter->second.empty()){
            throw InternalProgramError(
                &logger, PA_CURRENT_FUNCTION,
                "Sprite map is empty for slug: " + item.second
            );
        }
        item.second = *iter->second.begin();
    }

    result.log(logger, max_alpha);
    result.clear_beyond_alpha(max_alpha);

    return result;
}
std::string read_pokemon_sprite_with_item(
    Logger& logger,
    const ImageViewRGB32& screen, const ImageFloatBox& box
){
    using namespace papkmnlib;

    ImageMatch::ImageMatchResult result = read_pokemon_sprite_set_with_item(logger, screen, box);
    auto iter = result.results.begin();
    if (iter == result.results.end()){
        return "";
    }
    if (iter->first > SpeciesReadDatabase::CROPPED_MAX_ALPHA){
        logger.log("No good sprite match found.", COLOR_RED);
        return "";
    }
    if (result.results.size() > 2){
        logger.log("Multiple \"ok\" sprite matches with no clear winner.", COLOR_RED);
        return "";
    }

    const std::map<std::string, Pokemon>& RENTALS = all_rental_pokemon();
    if (RENTALS.find(iter->second) == RENTALS.end()){
        logger.log("Read " + STRING_POKEMON + " sprite is not a valid rental.", COLOR_RED);
        return "";
    }

    return std::move(iter->second);
}


std::string read_pokemon_name_sprite(
    Logger& logger,
    OcrFailureWatchdog& ocr_watchdog,
    const ImageViewRGB32& screen,
    const ImageFloatBox& sprite_box,
    const ImageFloatBox& name_box, Language language,
    bool allow_exact_match_fallback
){
    using namespace papkmnlib;

    const std::map<std::string, Pokemon>& RENTALS = all_rental_pokemon();

    ImageViewRGB32 image = extract_box_reference(screen, name_box);

    std::set<std::string> ocr_slugs;
    for (const std::string& slug : read_pokemon_name(logger, language, ocr_watchdog, image)){
        //  Only include candidates that are valid rental Pokemon.
        auto iter = RENTALS.find(slug);
        if (iter != RENTALS.end()){
            ocr_slugs.insert(slug);
        }
    }
    bool ocr_hit = !ocr_slugs.empty();
    bool ocr_unique = ocr_hit && ocr_slugs.size() == 1;
//    if (!ocr_hit){
//        dump_image(logger, MODULE_NAME, "MaxLair-read_name_sprite", screen);
//    }

    ImageMatch::ImageMatchResult result = read_pokemon_sprite_set(
        logger,
        screen,
        sprite_box,
        allow_exact_match_fallback
    );
    auto iter = result.results.begin();
    bool sprite_hit = iter != result.results.end() && iter->first <= SpeciesReadDatabase::CROPPED_MAX_ALPHA;
//    if (!sprite_hit || result.results.size() > 1){
//        dump_image(logger, MODULE_NAME, "MaxLair-read_name_sprite", screen);
//    }

    //  No hit on sprite. Use OCR.
    if (!sprite_hit){
        std::string ret = ocr_unique ? *ocr_slugs.begin() : "";
        logger.log("Failed to read sprite. Using OCR result: " + ret, COLOR_RED);
//        dump_image(logger, screen, "MaxLair-read_name_sprite");
        return ret;
    }

    //  No hit on OCR. Use sprite.
    if (!ocr_hit){
        std::string ret = sprite_hit ? std::move(iter->second) : "";
        logger.log("Failed to read name. Using sprite result: " + ret, COLOR_RED);
        ocr_watchdog.push_result(false);
//        dump_image(logger, screen, "MaxLair-read_name_sprite");
        return ret;
    }

    //  If any are in common to both, pick the best one.
    std::multimap<double, std::string> common;
    for (const auto& item : result.results){
        if (ocr_slugs.find(item.second) != ocr_slugs.end()){
            common.emplace(item.first, item.second);
        }
    }
    if (!common.empty()){
        logger.log("Sprite and OCR agree. Picking best one: " + common.begin()->second, COLOR_BLUE);
        return std::move(common.begin()->second);
    }

    //  This is where things get bad since OCR and sprites disagree.

    //  If there is only one sprite match, use it.
    if (result.results.size() == 1){
        logger.log("Sprite and OCR disagree. Attempt to arbitrate... Picking: " + result.results.begin()->second, COLOR_RED);
        ocr_watchdog.push_result(false);
        return std::move(result.results.begin()->second);
    }

    //  If there is only one OCR match, use it.
    if (ocr_slugs.size() == 1){
        logger.log("Sprite and OCR disagree. Attempt to arbitrate... Picking: " + *ocr_slugs.begin(), COLOR_RED);
        return *ocr_slugs.begin();
    }

    //  At this point, both OCR and sprites have multiple items in completely disjoint sets.
    logger.log("Sprite and OCR disagree so badly that no arbitration will be attempted.", COLOR_RED);
    ocr_watchdog.push_result(false);

    dump_image(logger, MODULE_NAME, "MaxLair-read_name_sprite", screen);

    return "";
}





}
}
}
}
