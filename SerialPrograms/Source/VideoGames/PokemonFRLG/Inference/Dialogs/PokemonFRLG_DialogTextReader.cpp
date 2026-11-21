/*  Dialog Text Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cctype>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_OcrPreprocessing.h"
#include "PokemonFRLG_DialogTextReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

//  Both dialog boxes (overworld and battle) put their two lines here.
const ImageFloatBox DIALOG_LINE_BOXES[2] = {
    {0.055, 0.755, 0.885, 0.088},
    {0.055, 0.850, 0.885, 0.088},
};

//  Party action list: lines are 16 GBA pixels (0.1 of the screen) apart and
//  the window grows upward from the bottom. Each box starts right of the
//  selection arrow.
constexpr double ACTION_BOTTOM_LINE_CENTER = 0.906;
constexpr double ACTION_LINE_SPACING = 0.100;
constexpr double ACTION_LINE_HEIGHT = 0.070;
constexpr double ACTION_LEFT = 0.665;
constexpr double ACTION_WIDTH = 0.300;
constexpr size_t ACTION_MAX_LINES = 8;   //  SUMMARY + 4 field moves + 3.

//  Gray system text and blue field-move text, both on white.
const std::vector<OCR::TextColorRange>& MENU_TEXT_FILTERS(){
    static const std::vector<OCR::TextColorRange> filters{
        {combine_rgb(0, 0, 0), combine_rgb(140, 140, 255)},
        {combine_rgb(0, 0, 0), combine_rgb(170, 170, 255)},
    };
    return filters;
}

const OCR::SmallDictionaryMatcher& action_dictionary(){
    static const OCR::SmallDictionaryMatcher matcher = []{
        const std::vector<std::pair<const char*, const char*>> actions{
            {"summary",     "SUMMARY"},
            {"switch",      "SWITCH"},
            {"item",        "ITEM"},
            {"mail",        "MAIL"},
            {"cancel",      "CANCEL"},
            {"flash",       "FLASH"},
            {"cut",         "CUT"},
            {"fly",         "FLY"},
            {"strength",    "STRENGTH"},
            {"surf",        "SURF"},
            {"rock-smash",  "ROCK SMASH"},
            {"waterfall",   "WATERFALL"},
            {"teleport",    "TELEPORT"},
            {"dig",         "DIG"},
            {"milk-drink",  "MILK DRINK"},
            {"softboiled",  "SOFTBOILED"},
            {"sweet-scent", "SWEET SCENT"},
        };
        JsonObject eng;
        for (const auto& item : actions){
            JsonArray names;
            names.push_back(std::string(item.second));
            eng[item.first] = std::move(names);
        }
        JsonObject root;
        root["eng"] = std::move(eng);
        return OCR::SmallDictionaryMatcher(root);
    }();
    return matcher;
}

}


std::string read_dialog_line(
    Logger& logger, const ImageViewRGB32& frame,
    DialogStyle style, int line
){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 box = extract_box_reference(game_screen, DIALOG_LINE_BOXES[line == 0 ? 0 : 1]);

    //  Text becomes black on white.
    ImageRGB32 bw = style == DialogStyle::BATTLE
        ? to_blackwhite_rgb32_range(box, true, combine_rgb(200, 200, 200), combine_rgb(255, 255, 255))
        : to_blackwhite_rgb32_range(box, true, combine_rgb(0, 0, 0), combine_rgb(150, 150, 255));

    //  Uses whichever OCR library is selected in the global settings.
    std::string text = OCR::ocr_read(Language::English, bw);
    for (char& ch : text){
        ch = (char)std::tolower((unsigned char)ch);
    }
    while (!text.empty() && (text.back() == '\n' || text.back() == ' ')){
        text.pop_back();
    }
    logger.log("Dialog line " + std::to_string(line + 1) + ": \"" + text + "\"");
    if (text.empty() && GlobalSettings::instance().SAVE_DEBUG_IMAGES){
        box.save("DebugDumps/ocr_dialog_line.png");
    }
    return text;
}

bool text_contains_any(const std::string& text, const std::vector<std::string>& keywords){
    for (const std::string& keyword : keywords){
        if (text.find(keyword) != std::string::npos){
            return true;
        }
    }
    return false;
}

int last_number_in(const std::string& text){
    size_t end = text.size();
    while (end > 0 && !std::isdigit((unsigned char)text[end - 1])){
        end--;
    }
    if (end == 0){
        return -1;
    }
    size_t start = end;
    while (start > 0 && std::isdigit((unsigned char)text[start - 1])){
        start--;
    }
    if (end - start > 4){
        return -1;
    }
    return std::stoi(text.substr(start, end - start));
}


std::string read_bag_description(Logger& logger, const ImageViewRGB32& frame){
    //  Two lines of white text on blue, right of the item's picture. The OCR
    //  reads one line at a time: this is the second line, (40, 129) to
    //  (228, 145) of the 240x160 GBA screen, which has the number of steps
    //  for the Repels ("appearing for 250 steps.").
    const ImageFloatBox DESCRIPTION_BOX(40.0 / 240, 129.0 / 160, 188.0 / 240, 16.0 / 160);
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 box = extract_box_reference(game_screen, DESCRIPTION_BOX);

    ImageRGB32 bw = to_blackwhite_rgb32_range(box, true, combine_rgb(190, 190, 190), combine_rgb(255, 255, 255));
    std::string text = OCR::ocr_read(Language::English, bw);
    for (char& ch : text){
        ch = ch == '\n' ? ' ' : (char)std::tolower((unsigned char)ch);
    }
    while (!text.empty() && text.back() == ' '){
        text.pop_back();
    }
    logger.log("Bag item description: \"" + text + "\"");
    if (text.empty() && GlobalSettings::instance().SAVE_DEBUG_IMAGES){
        box.save("DebugDumps/ocr_bag_description.png");
    }
    return text;
}

std::string read_fly_map_name(Logger& logger, const ImageViewRGB32& frame){
    //  The name window is at (24, 16) to (144, 32) of the 240x160 GBA screen.
    const ImageFloatBox NAME_BOX(24.0 / 240, 16.0 / 160, 120.0 / 240, 16.0 / 160);
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 box = extract_box_reference(game_screen, NAME_BOX);

    //  White text becomes black on white; the map behind it doesn't.
    ImageRGB32 bw = to_blackwhite_rgb32_range(box, true, combine_rgb(190, 190, 190), combine_rgb(255, 255, 255));
    std::string raw = OCR::ocr_read(Language::English, bw);
    std::string text;
    for (char ch : raw){
        if (std::isalpha((unsigned char)ch) || ch == ' '){
            text += (char)std::tolower((unsigned char)ch);
        }
    }
    while (!text.empty() && text.back() == ' '){
        text.pop_back();
    }
    logger.log("Fly map cursor on: \"" + text + "\"");
    if (text.empty() && GlobalSettings::instance().SAVE_DEBUG_IMAGES){
        box.save("DebugDumps/ocr_fly_map_name.png");
    }
    return text;
}


std::vector<std::string> read_party_action_list(Logger& logger, const ImageViewRGB32& frame){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    //  Read from the bottom (CANCEL) up until SUMMARY, which is always first.
    std::vector<std::string> bottom_up;
    for (size_t i = 0; i < ACTION_MAX_LINES; i++){
        double center = ACTION_BOTTOM_LINE_CENTER - ACTION_LINE_SPACING * (double)i;
        double top = center - ACTION_LINE_HEIGHT / 2;
        if (top < 0){
            break;
        }
        ImageFloatBox box(ACTION_LEFT, top, ACTION_WIDTH, ACTION_LINE_HEIGHT);
        ImageRGB32 ready = preprocess_for_ocr(extract_box_reference(game_screen, box));
        OCR::StringMatchResult result = action_dictionary().match_substring_from_image_multifiltered(
            &logger, Language::English, ready,
            MENU_TEXT_FILTERS(),
            -1.40, 0.50
        );
        std::string token = result.results.empty() ? "" : result.results.begin()->second.token;
        bottom_up.emplace_back(token);
        if (token == "summary"){
            break;
        }
    }

    std::vector<std::string> ret(bottom_up.rbegin(), bottom_up.rend());
    std::string log = "Party action list:";
    for (const std::string& token : ret){
        log += " [" + (token.empty() ? std::string("?") : token) + "]";
    }
    logger.log(log);
    return ret;
}



}
}
}
