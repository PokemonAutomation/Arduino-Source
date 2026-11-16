/*  Battle Move Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_OcrPreprocessing.h"
#include "PokemonFRLG_BattleMoveReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


const BattleMoveOCR& BattleMoveOCR::instance(){
    static BattleMoveOCR reader;
    return reader;
}
BattleMoveOCR::BattleMoveOCR()
    : SmallDictionaryMatcher("PokemonFRLG/BattleMovesOCR.json")
{}


BattleMoveReader::BattleMoveReader(Language language, Color color)
    : m_language(language)
    , m_color(color)
    //  Each box starts right of where the selection arrow is drawn.
    , m_move_boxes{{
        {0.062, 0.765, 0.280, 0.075},   //  top-left
        {0.364, 0.765, 0.280, 0.075},   //  top-right
        {0.062, 0.862, 0.280, 0.075},   //  bottom-left
        {0.364, 0.862, 0.280, 0.075},   //  bottom-right
    }}
    //  The name in the player's HP box. Ends before "Lv".
    , m_own_name_box(0.585, 0.482, 0.245, 0.065)
    //  "Lv" and the level in the player's HP box, up to the box's border.
    //  The level is right aligned: "Lv100" starts 5 GBA pixels left of "Lv99".
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
        OCR::StringMatchResult result = BattleMoveOCR::instance().match_substring_from_image_multifiltered(
            &logger, m_language, ready,
            DARK_TEXT_FILTERS(),
            BattleMoveOCR::MAX_LOG10P, BattleMoveOCR::MAX_LOG10P_SPREAD
        );
        if (!result.results.empty()){
            ret[i] = result.results.begin()->second.token;
            continue;
        }
        logger.log("Move slot " + std::to_string(i + 1) + ": empty or unreadable.");
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            box.save("DebugDumps/ocr_move_box_" + std::to_string(i + 1) + ".png");
        }
    }
    return ret;
}

std::string BattleMoveReader::read_own_species(
    Logger& logger, const ImageViewRGB32& frame,
    const std::set<std::string>& subset
) const{
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    ImageViewRGB32 box = extract_box_reference(game_screen, m_own_name_box);
    ImageRGB32 ready = preprocess_for_ocr(box);
    OCR::StringMatchResult result = subset.empty()
        ? Pokemon::PokemonNameReader::instance().read_substring(logger, m_language, ready, DARK_TEXT_FILTERS())
        : Pokemon::PokemonNameReader(subset).read_substring(logger, m_language, ready, DARK_TEXT_FILTERS());
    if (result.results.empty()){
        logger.log("Could not read the species of the battling Pokemon.", COLOR_ORANGE);
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            box.save("DebugDumps/ocr_own_name_box.png");
        }
        return "";
    }
    return result.results.begin()->second.token;
}

namespace{

//  Dark text on the white HP box becomes black on white, then raw OCR.
std::string ocr_dark_text(Language language, const ImageViewRGB32& box){
    ImageRGB32 bw = to_blackwhite_rgb32_range(box, true, combine_rgb(0, 0, 0), combine_rgb(130, 130, 130));
    std::string text = OCR::ocr_read(language, bw);
    while (!text.empty() && (text.back() == '\n' || text.back() == ' ')){
        text.pop_back();
    }
    return text;
}

}

std::string BattleMoveReader::read_own_name_text(Logger& logger, const ImageViewRGB32& frame) const{
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    std::string text = ocr_dark_text(m_language, extract_box_reference(game_screen, m_own_name_box));
    logger.log("Battling Pokemon's name: \"" + text + "\"");
    return text;
}

namespace{

//  The digits of the small font (graphics/fonts/latin_small.png in the
//  decomp), 4 x 7 pixels, the top row first, the leftmost pixel as the
//  highest bit.
const uint8_t LEVEL_DIGITS[10][7] = {
    {0b0110, 0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110},     //  0
    {0b0010, 0b0110, 0b0010, 0b0010, 0b0010, 0b0010, 0b0111},     //  1
    {0b0110, 0b1001, 0b0001, 0b0010, 0b0100, 0b1000, 0b1111},     //  2
    {0b0110, 0b1001, 0b0001, 0b0110, 0b0001, 0b1001, 0b0110},     //  3
    {0b0110, 0b1010, 0b1010, 0b1010, 0b1010, 0b1111, 0b0010},     //  4
    {0b1111, 0b1000, 0b1000, 0b1110, 0b0001, 0b1001, 0b0110},     //  5
    {0b0110, 0b1001, 0b1000, 0b1110, 0b1001, 0b1001, 0b0110},     //  6
    {0b1111, 0b0001, 0b0001, 0b0010, 0b0010, 0b0100, 0b0100},     //  7
    {0b0110, 0b1001, 0b1001, 0b0110, 0b1001, 0b1001, 0b0110},     //  8
    {0b0110, 0b1001, 0b1001, 0b0111, 0b0001, 0b1001, 0b0110},     //  9
};

bool is_dark(uint32_t pixel){
    return ((pixel >> 16) & 0xff) <= 130 && ((pixel >> 8) & 0xff) <= 130 && (pixel & 0xff) <= 130;
}

}

int BattleMoveReader::read_own_level(Logger& logger, const ImageViewRGB32& frame) const{
    //  The HP box shows a level label and then the level, right aligned: the
    //  digits are 5 pixels apart and the last one always ends at the same
    //  place. The label depends on the language ("Lv" in English, "N." in
    //  French, ...), so it isn't read: each digit is matched against the
    //  font's digits from the right, and the first character that isn't a
    //  digit ends the number.
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);
    if (game_screen.width() == 0 || game_screen.height() == 0){
        return -1;
    }
    auto dark = [&](int x, int y){
        size_t px = (size_t)((x + 0.5) * (double)game_screen.width() / 240);
        size_t py = (size_t)((y + 0.5) * (double)game_screen.height() / 160);
        return is_dark(game_screen.pixel(px, py));
    };

    //  The level's area of the HP box, in GBA pixels, inside its border.
    const int LEFT = 196;
    const int RIGHT = 222;
    const int TOP = 78;
    const int BOTTOM = 89;

    //  The last digit's right edge (every digit has a pixel in its last
    //  column), then its top row (and in its first row).
    int right = -1;
    for (int x = RIGHT - 1; x >= LEFT && right < 0; x--){
        for (int y = TOP; y < BOTTOM; y++){
            if (dark(x, y)){
                right = x;
                break;
            }
        }
    }
    if (right < LEFT + 3){
        logger.log("Could not find the battling Pokemon's level.", COLOR_ORANGE);
        return -1;
    }
    int top = -1;
    for (int y = TOP; y < BOTTOM && top < 0; y++){
        for (int x = right - 3; x <= right; x++){
            if (dark(x, y)){
                top = y;
                break;
            }
        }
    }
    if (top < 0 || top + 7 > BOTTOM){
        logger.log("Could not find the battling Pokemon's level.", COLOR_ORANGE);
        return -1;
    }

    //  Up to 3 digits, from the right.
    int level = 0;
    int scale = 1;
    int digits = 0;
    std::string distances;
    for (int k = 0; k < 3; k++){
        int left = right - 3 - 5 * k;
        if (left < LEFT){
            break;
        }
        int best_digit = -1;
        int best_distance = 99;
        for (int d = 0; d < 10; d++){
            int distance = 0;
            for (int row = 0; row < 7; row++){
                for (int col = 0; col < 4; col++){
                    bool expected = ((LEVEL_DIGITS[d][row] >> (3 - col)) & 1) != 0;
                    if (dark(left + col, top + row) != expected){
                        distance++;
                    }
                }
            }
            if (distance < best_distance){
                best_distance = distance;
                best_digit = d;
            }
        }
        //  A few pixels off at most; the label's characters are far off.
        if (best_distance > 3){
            break;
        }
        level += best_digit * scale;
        scale *= 10;
        digits++;
        distances += (distances.empty() ? "" : ", ") + std::to_string(best_distance);
    }

    if (digits == 0 || level < 1 || level > 100){
        logger.log("Could not read the battling Pokemon's level.", COLOR_ORANGE);
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            extract_box_reference(game_screen, m_own_level_box).save("DebugDumps/own_level_box.png");
        }
        return -1;
    }
    logger.log("Battling Pokemon's level: " + std::to_string(level) + " (pixels off per digit: " + distances + ")");
    return level;
}


//  A battle screen (the battle menu, the move list or a message) with the
//  player's Pokemon's HP box: its level, and, if given, its species and the
//  four moves of the move list (a "" move is an empty slot).
struct BattleMoveReaderCase{
    std::string image;
    int level;
    std::string species = "";
    std::set<std::string> species_subset = {};
    bool move_list = false;
    std::array<std::string, 4> moves = {};
};

class Test_BattleMoveReader : public ImageUnitTest{
public:
    Test_BattleMoveReader(BattleMoveReaderCase test)
        : ImageUnitTest("PokemonFRLG::BattleMoveReader - " + test.image, {test.image})
        , m_case(std::move(test))
    {
        //  Plus the PaddleOCR (ONNX) recognition session and its working set.
        m_memory += PADDLE_OCR_SINGLE_LINE_MEMORY_ESTIMATE;
    }

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        BattleMoveReader reader(Language::English);
        TEST_RESULT_COMPONENT_EQUAL_STR(reader.read_own_level(logger, images[0]), m_case.level, "level");
        if (!m_case.species.empty()){
            TEST_RESULT_COMPONENT_EQUAL_STR(
                reader.read_own_species(logger, images[0], m_case.species_subset),
                m_case.species, "species"
            );
        }
        if (m_case.move_list){
            std::array<std::string, 4> moves = reader.read_moves(logger, images[0]);
            for (size_t i = 0; i < 4; i++){
                TEST_RESULT_COMPONENT_EQUAL_STR(
                    "\"" + moves[i] + "\"", "\"" + m_case.moves[i] + "\"",
                    "move " + std::to_string(i + 1)
                );
            }
        }
        return true;
    }

private:
    BattleMoveReaderCase m_case;
};

void add_tests_BattleMoveReader(UnitTestDatabase& database){
    const std::string FOLDER = "PokemonFRLG/BattleMoveReader/";
    const BattleMoveReaderCase CASES[] = {
        {
            .image = FOLDER + "Nickname-Lv5-Moves.jpg", .level = 5,
            .move_list = true, .moves = {"barrier", "confusion", "", ""},
        },
        {.image = FOLDER + "Nickname-Lv5-Menu.jpg", .level = 5},
        {
            .image = FOLDER + "Pidgeot-Lv78-Moves.jpg", .level = 78, .species = "pidgeot",
            .move_list = true, .moves = {"wing-attack", "agility", "fly", "quick-attack"},
        },
        {
            .image = FOLDER + "Lapras-Lv81-Moves.jpg", .level = 81, .species = "lapras",
            .move_list = true, .moves = {"surf", "ice-beam", "psychic", "thunderbolt"},
        },
        {.image = FOLDER + "Lapras-Lv81-Menu.jpg", .level = 81, .species = "lapras"},
        //  Among all species, this MEWTWO is read as MEOWTH; the EXP Farmer
        //  only expects Mewtwo or Lapras.
        {
            .image = FOLDER + "Mewtwo-Lv100-Message.jpg", .level = 100, .species = "mewtwo",
            .species_subset = {"mewtwo", "lapras"},
        },
        {
            .image = FOLDER + "Charizard-Lv100-Moves.jpg", .level = 100, .species = "charizard",
            .move_list = true, .moves = {"fire-blast", "hidden-power", "blast-burn", "substitute"},
        },
    };
    for (const BattleMoveReaderCase& test : CASES){
        database.add<Test_BattleMoveReader>(test);
    }
}


}
}
}
