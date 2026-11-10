/*  Battle Move Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cctype>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "VideoGames/PokemonFRLG/Programs/Farming/PokemonFRLG_BattleMoveData.h"
#include "PokemonFRLG_OcrPreprocessing.h"
#include "PokemonFRLG_BattleMoveReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

//  OCR dictionary of every Gen 3 move, built from the move table.
const OCR::SmallDictionaryMatcher& move_dictionary(){
    static const OCR::SmallDictionaryMatcher matcher = []{
        size_t count = 0;
        const MoveInfo* moves = all_moves(count);
        JsonObject eng;
        for (size_t i = 0; i < count; i++){
            JsonArray names;
            names.push_back(std::string(moves[i].display_name));
            eng[moves[i].slug] = std::move(names);
        }
        JsonObject root;
        root["eng"] = std::move(eng);
        return OCR::SmallDictionaryMatcher(root);
    }();
    return matcher;
}

}


BattleMoveReader::BattleMoveReader(Color color)
    : m_color(color)
    //  Each box starts right of where the selection arrow is drawn.
    , m_move_boxes{{
        {0.062, 0.765, 0.280, 0.075},   //  top-left
        {0.364, 0.765, 0.280, 0.075},   //  top-right
        {0.062, 0.862, 0.280, 0.075},   //  bottom-left
        {0.364, 0.862, 0.280, 0.075},   //  bottom-right
    }}
    //  Name in the player's HP box. Ends before "Lv".
    , m_own_name_box(0.585, 0.482, 0.245, 0.065)
    //  "Lv" and the level in the player's HP box, up to the box's border.
    //  The text is right aligned: "Lv100" starts 5 GBA pixels left of "Lv99".
    , m_own_level_box(0.818, 0.480, 0.112, 0.066)
{}

void BattleMoveReader::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    for (const ImageFloatBox& box : m_move_boxes){
        items.add(m_color, GAME_BOX.inner_to_outer(box));
    }
    items.add(m_color, GAME_BOX.inner_to_outer(m_own_name_box));
    items.add(m_color, GAME_BOX.inner_to_outer(m_own_level_box));
}

std::array<std::string, 4> BattleMoveReader::read_moves(Logger& logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    std::array<std::string, 4> ret;
    for (size_t i = 0; i < 4; i++){
        ImageViewRGB32 box = extract_box_reference(game_screen, m_move_boxes[i]);
        ImageRGB32 ready = preprocess_for_ocr(box);
        OCR::StringMatchResult result = move_dictionary().match_substring_from_image_multifiltered(
            &logger, Language::English, ready,
            DARK_TEXT_FILTERS(),
            -1.40, 0.50
        );
        if (!result.results.empty()){
            ret[i] = result.results.begin()->second.token;
        }else{
            logger.log("Move slot " + std::to_string(i + 1) + ": empty or unreadable.");
            if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
                box.save("DebugDumps/ocr_move_box_" + std::to_string(i + 1) + ".png");
            }
        }
    }
    return ret;
}

std::string BattleMoveReader::read_own_species(Logger& logger, Language language, const ImageViewRGB32& frame) const{
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 box = extract_box_reference(game_screen, m_own_name_box);
    ImageRGB32 ready = preprocess_for_ocr(box);
    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        logger, language, ready, DARK_TEXT_FILTERS()
    );
    if (result.results.empty()){
        logger.log("Could not read the name of the battling Pokemon.", COLOR_RED);
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            box.save("DebugDumps/ocr_own_name_box.png");
        }
        return "";
    }
    return result.results.begin()->second.token;
}

namespace{
//  Dark text on the white HP box becomes black on white, then raw OCR.
std::string ocr_dark_text(const ImageViewRGB32& box){
    ImageRGB32 bw = to_blackwhite_rgb32_range(box, true, combine_rgb(0, 0, 0), combine_rgb(130, 130, 130));
    return OCR::ocr_read(Language::English, bw);
}
}

std::string BattleMoveReader::read_own_name_text(Logger& logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    std::string raw = ocr_dark_text(extract_box_reference(game_screen, m_own_name_box));
    std::string text;
    for (char ch : raw){
        if (std::isalpha((unsigned char)ch)){
            text += (char)std::tolower((unsigned char)ch);
        }
    }
    logger.log("Battling Pokemon's name: \"" + text + "\"");
    return text;
}

int BattleMoveReader::read_own_level(Logger& logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 box = extract_box_reference(game_screen, m_own_level_box);
    std::string raw = ocr_dark_text(box);
    //  Only what comes after the "v" of "Lv", if it was read.
    size_t v = raw.find_last_of("vV");
    std::string after = v == std::string::npos ? raw : raw.substr(v + 1);
    std::string digits;
    for (char ch : after){
        if (ch == 'o' || ch == 'O'){
            ch = '0';
        }
        if (std::isdigit((unsigned char)ch)){
            digits += ch;
        }
    }
    int level = digits.empty() || digits.size() > 3 ? -1 : std::stoi(digits);
    if (level < 1 || level > 100){
        logger.log("Could not read the battling Pokemon's level: \"" + raw + "\"", COLOR_ORANGE);
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            box.save("DebugDumps/ocr_own_level_box.png");
        }
        return -1;
    }
    logger.log("Battling Pokemon's level: " + std::to_string(level));
    return level;
}



}
}
}
