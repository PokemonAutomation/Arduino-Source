/*  Dialog Text Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <ctype.h>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
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

//  The bag's item description: two lines of white text on blue, right of the
//  item's picture, at (40, 113) and (40, 129) of the 240x160 GBA screen.
const ImageFloatBox BAG_DESCRIPTION_LINE_BOXES[2] = {
    {40.0 / 240, 113.0 / 160, 188.0 / 240, 16.0 / 160},
    {40.0 / 240, 129.0 / 160, 188.0 / 240, 16.0 / 160},
};

//  The Fly map's place name window, at (24, 16) to (144, 32) of the GBA screen.
const ImageFloatBox FLY_MAP_NAME_BOX(24.0 / 240, 16.0 / 160, 120.0 / 240, 16.0 / 160);

//  The text of one line of a text window: `box` in the game screen, with the
//  text's colors between `min_color` and `max_color`. Line breaks become
//  spaces and the ends are trimmed. (The OCR reads a single line.)
std::string read_text_line(
    Logger& logger, Language language, const ImageViewRGB32& frame,
    const ImageFloatBox& box, uint32_t min_color, uint32_t max_color,
    const std::string& what, const char* debug_image
){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 image = extract_box_reference(game_screen, box);

    //  The text becomes black on white.
    ImageRGB32 bw = to_blackwhite_rgb32_range(image, true, min_color, max_color);
    std::string text = OCR::ocr_read(language, bw);
    for (char& ch : text){
        if (ch == '\n' || ch == '\r'){
            ch = ' ';
        }
    }
    size_t start = 0;
    while (start < text.size() && text[start] == ' '){
        start++;
    }
    size_t end = text.size();
    while (end > start && text[end - 1] == ' '){
        end--;
    }
    text = text.substr(start, end - start);

    logger.log(what + ": \"" + text + "\"");
    if (text.empty() && GlobalSettings::instance().SAVE_DEBUG_IMAGES){
        image.save(debug_image);
    }
    return text;
}

}


std::string read_dialog_line(
    Logger& logger, Language language, const ImageViewRGB32& frame,
    DialogStyle style, int line
){
    const ImageFloatBox& box = DIALOG_LINE_BOXES[line == 0 ? 0 : 1];
    std::string what = "Dialog line " + std::to_string(line == 0 ? 1 : 2);
    if (style == DialogStyle::BATTLE){
        return read_text_line(
            logger, language, frame, box,
            combine_rgb(200, 200, 200), combine_rgb(255, 255, 255),
            what, "DebugDumps/ocr_dialog_line.png"
        );
    }else{
        return read_text_line(
            logger, language, frame, box,
            combine_rgb(0, 0, 0), combine_rgb(150, 150, 255),
            what, "DebugDumps/ocr_dialog_line.png"
        );
    }
}

std::string read_fly_map_name(Logger& logger, Language language, const ImageViewRGB32& frame){
    //  White text. The map behind it doesn't pass the filter.
    return read_text_line(
        logger, language, frame, FLY_MAP_NAME_BOX,
        combine_rgb(190, 190, 190), combine_rgb(255, 255, 255),
        "Fly map cursor on", "DebugDumps/ocr_fly_map_name.png"
    );
}

std::string read_bag_description(Logger& logger, Language language, const ImageViewRGB32& frame){
    std::string text;
    for (const ImageFloatBox& box : BAG_DESCRIPTION_LINE_BOXES){
        std::string line = read_text_line(
            logger, language, frame, box,
            combine_rgb(190, 190, 190), combine_rgb(255, 255, 255),
            "Bag item description line", "DebugDumps/ocr_bag_description.png"
        );
        if (line.empty()){
            continue;
        }
        if (!text.empty()){
            text += ' ';
        }
        text += line;
    }
    return text;
}


bool text_contains_any(const std::string& text, const std::vector<std::string>& words){
    std::u32string normalized = OCR::normalize_utf32(text);
    for (const std::string& word : words){
        std::u32string normalized_word = OCR::normalize_utf32(word);
        if (!normalized_word.empty() && normalized.find(normalized_word) != std::u32string::npos){
            return true;
        }
    }
    return false;
}

int last_number_in(const std::string& text){
    size_t end = text.size();
    while (end > 0 && !isdigit((unsigned char)text[end - 1])){
        end--;
    }
    if (end == 0){
        return -1;
    }
    size_t start = end;
    while (start > 0 && isdigit((unsigned char)text[start - 1])){
        start--;
    }
    if (end - start > 4){
        return -1;
    }
    return std::stoi(text.substr(start, end - start));
}



}
}
}
