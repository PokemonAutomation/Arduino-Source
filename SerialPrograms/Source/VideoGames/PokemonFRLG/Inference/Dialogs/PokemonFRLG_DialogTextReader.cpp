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
#include "Tests/ImageUnitTest.h"
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


//  Read one line of a dialog box; it must contain the expected words.
class Test_DialogTextReader : public ImageUnitTest{
public:
    Test_DialogTextReader(const std::string& image, DialogStyle style, int line, std::vector<std::string> words)
        : ImageUnitTest("PokemonFRLG::DialogTextReader - line " + std::to_string(line) + " - " + image, {image})
        , m_style(style), m_line(line), m_words(std::move(words))
    {
        //  Plus the PaddleOCR (ONNX) recognition session and its working set.
        m_memory += PADDLE_OCR_SINGLE_LINE_MEMORY_ESTIMATE;
    }

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        std::string text = read_dialog_line(logger, Language::English, images[0], m_style, m_line);
        for (const std::string& word : m_words){
            if (!text_contains_any(text, {word})){
                return "Read \"" + text + "\", which doesn't contain \"" + word + "\".";
            }
        }
        return true;
    }

private:
    DialogStyle m_style;
    int m_line;
    std::vector<std::string> m_words;
};

//  text_contains_any() and last_number_in(), without images.
class Test_DialogTextHelpers : public UnitTest{
public:
    Test_DialogTextHelpers()
        : UnitTest("PokemonFRLG::DialogTextReader - text helpers")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        if (!text_contains_any("Your POK\xC3\xA9MON were healed!", {"pokemon"})){
            return "\"POK\xC3\xA9MON\" should match \"pokemon\".";
        }
        if (text_contains_any("SIX ISLAND", {"seven", "one island"})){
            return "\"SIX ISLAND\" should match neither \"seven\" nor \"one island\".";
        }
        const std::pair<const char*, int> NUMBERS[] = {
            {"The VS SEEKER can be used again after 100 steps.", 100},
            {"Lv5", 5},
            {"No number here.", -1},
            {"123456", -1},
        };
        for (const auto& item : NUMBERS){
            if (last_number_in(item.first) != item.second){
                return std::string("last_number_in(\"") + item.first + "\") should be " + std::to_string(item.second) + ".";
            }
        }
        return true;
    }
};

void add_tests_DialogTextReader(UnitTestDatabase& database){
    database.add<Test_DialogTextReader>(
        "PokemonFRLG/DialogTextReader/SixIsland-Sign.jpg", DialogStyle::OVERWORLD, 0,
        std::vector<std::string>{"six island"}
    );
    database.add<Test_DialogTextReader>(
        "PokemonFRLG/DialogTextReader/SixIsland-Sign.jpg", DialogStyle::OVERWORLD, 1,
        std::vector<std::string>{"fortune", "wisdom"}
    );
    database.add<Test_DialogTextReader>(
        "PokemonFRLG/DialogTextReader/WildOddish-StunSpore.jpg", DialogStyle::BATTLE, 0,
        std::vector<std::string>{"oddish", "used"}
    );
    database.add<Test_DialogTextReader>(
        "PokemonFRLG/DialogTextReader/WildOddish-StunSpore.jpg", DialogStyle::BATTLE, 1,
        std::vector<std::string>{"stun spore"}
    );
    database.add<Test_DialogTextHelpers>();
}


}
}
}
