/*  Pokemon Home Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Strings/Unicode.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_DictionaryMatcher.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "Pokemon/Inference/Pokemon_AbilityReader.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"
#include "PokemonHome_SummaryReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

namespace{

class LanguageOfOriginMatcher : public OCR::DictionaryMatcher{
public:
    LanguageOfOriginMatcher(){
        const std::string path = RESOURCE_PATH() + "Pokemon/LanguageOfOrigin.json";
        JsonValue json = load_json_file(path);
        const JsonObject& dictionary = json.to_object_throw(path);
        m_database.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(Language::English),
            std::forward_as_tuple(
                dictionary, nullptr,
                language_data(Language::English).random_match_chance,
                false
            )
        );
        m_languages += Language::English;
    }
};

const std::vector<std::pair<uint32_t, uint32_t>>& white_number_filters(){
    static const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0xff808080, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
    };
    return filters;
}

const std::vector<std::pair<uint32_t, uint32_t>>& gray_number_filters(){
    static const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0x323232, 0xffe1e8e4},
    };
    return filters;
}

const std::vector<BlackWhiteRgb32Range>& white_text_filters(){
    static const std::vector<BlackWhiteRgb32Range> filters = []{
        std::vector<BlackWhiteRgb32Range> ret;
        for (const OCR::TextColorRange& filter : OCR::WHITE_TEXT_FILTERS())
            ret.push_back({ true, filter.mins, filter.maxs });
        return ret;
        }();
    return filters;
}

const std::vector<BlackWhiteRgb32Range>& gray_text_filters(){
    static const std::vector<BlackWhiteRgb32Range> filters = {
        {true, 0x323232, 0xffe1e8e4},
    };
    return filters;
}

const std::vector<OCR::TextColorRange>& gray_nature_text_filters(){
    static const std::vector<OCR::TextColorRange> filters = {
        {0x323232, 0xffe1e8e4},
    };
    return filters;
}

std::string read_text(
    Language language,
    const ImageViewRGB32& screen, const ImageFloatBox& box,
    const std::vector<BlackWhiteRgb32Range>& filters
){
    if (language == Language::None)
        return "";

    std::string best_raw;
    for (auto& [image, pixel_count] : to_blackwhite_rgb32_range(extract_box_reference(screen, box), filters)){
        if (pixel_count == 0)
            continue;
        std::string candidate = OCR::ocr_read(language, image, OCR::PageSegMode::SINGLE_LINE);
        if (!candidate.empty()){
            best_raw = candidate;
            break;
        }
    }
    return utf32_to_str(OCR::normalize_utf32(best_raw));
}

int read_number(
    Logger& logger, const ImageViewRGB32& screen, const ImageFloatBox& box,
    const std::vector<std::pair<uint32_t, uint32_t>>& filters
){
    return OCR::read_number_waterfill_multifilter(
        logger,
        GlobalThreadPools::computation_normal(),
        extract_box_reference(screen, box),
        filters
    );
}

} //namespace

SummaryReader::SummaryReader(Color color)
    : m_color(color)
    , m_national_dex_number_box(0.448, 0.245, 0.049, 0.04)
    , m_level_box(0.506141, 0.090805, 0.100194, 0.057471)
    , m_original_trainer_id_box(0.782, 0.719, 0.193, 0.046)
    , m_original_trainer_name_box(0.492, 0.719, 0.165, 0.049)
    , m_nature_box(0.157, 0.783, 0.212, 0.042)
    , m_ability_box(0.158, 0.838, 0.213, 0.042)
    , m_language_of_origin_box(0.0265, 0.176, 0.056, 0.035)
{}

void SummaryReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_national_dex_number_box);
    items.add(m_color, m_level_box);
    items.add(m_color, m_original_trainer_id_box);
    items.add(m_color, m_original_trainer_name_box);
    items.add(m_color, m_nature_box);
    items.add(m_color, m_ability_box);
    items.add(m_color, m_language_of_origin_box);
}

int SummaryReader::read_national_dex(Logger& logger, const ImageViewRGB32& screen) const{
    return read_number(logger, screen, m_national_dex_number_box, white_number_filters());
}

int SummaryReader::read_original_trainer_id(Logger& logger, const ImageViewRGB32& screen) const{
    return read_number(logger, screen, m_original_trainer_id_box, white_number_filters());
}

std::string SummaryReader::read_original_trainer_name(Language language, const ImageViewRGB32& screen) const{
    return read_text(language, screen, m_original_trainer_name_box, white_text_filters());
}

std::string SummaryReader::read_nature(Logger& logger, Language language, const ImageViewRGB32& screen) const{
    static const Pokemon::NatureReader reader("Pokemon/NatureCheckerOCR.json");
    OCR::StringMatchResult result = reader.read_substring(
        logger,
        language,
        extract_box_reference(screen, m_nature_box),
        gray_nature_text_filters()
    );
    return result.results.size() == 1 ? result.results.begin()->second.token : "";
}

std::string SummaryReader::read_ability(Logger& logger, Language language, const ImageViewRGB32& screen) const{
    static const Pokemon::AbilityReader reader;
    OCR::StringMatchResult result = reader.read_substring(
        logger,
        language,
        extract_box_reference(screen, m_ability_box),
        gray_nature_text_filters()
    );
    return result.results.size() == 1 ? result.results.begin()->second.token : "";
}

std::string SummaryReader::read_language_of_origin(const ImageViewRGB32& screen) const{
    static const LanguageOfOriginMatcher dictionary;
    OCR::StringMatchResult result = dictionary.match_substring_from_image_multifiltered(
        nullptr,
        Language::English,
        extract_box_reference(screen, m_language_of_origin_box),
        OCR::BLACK_TEXT_FILTERS(),
        -1.30,
        0.50,
        0.01,
        0.50,
        OCR::PageSegMode::SINGLE_LINE
    );

    if (result.results.empty()){
        return "";
    }
    return result.results.begin()->second.token;
}

// Due to the position of the level changing slightly depending on digits and language. The box contains the "Lv" text and the level number.
int SummaryReader::read_level(Logger& logger, const ImageViewRGB32& screen) const{
    try{
        std::string text = read_text(Language::English, screen, m_level_box, gray_text_filters());
        //logger.log("Level text: " + text, COLOR_BLUE);
        size_t pos = text.find('v');

        if (pos != std::string::npos){
            std::string level = text.substr(pos + 1);
            return std::stoi(level);
        }

        return std::stoi(text.c_str());
    }
    catch (const std::exception&){
        logger.log("Failed to read level from summary screen.", COLOR_RED);
        return -1;
    }
}

class Test_SummaryReader_Numbers : public UnitTest{
public:
    Test_SummaryReader_Numbers(const std::string& image, int expected_dex, int expected_ot_id, int expected_level)
        : UnitTest("PokemonHome::SummaryReader_Numbers - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected_dex(expected_dex)
        , m_expected_ot_id(expected_ot_id)
        , m_expected_level(expected_level)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        SummaryReader reader;
        int dex = reader.read_national_dex(logger, image);
        int ot_id = reader.read_original_trainer_id(logger, image);
        int level = reader.read_level(logger, image);
        std::string errors;
        if (dex != m_expected_dex){
            errors += "National Dex number: Expected: " + std::to_string(m_expected_dex) + ", received: " + std::to_string(dex) + "\n";
        }
        if (ot_id != m_expected_ot_id){
            errors += "Original Trainer ID: Expected: " + std::to_string(m_expected_ot_id) + ", received: " + std::to_string(ot_id) + "\n";
        }
        if (level != m_expected_level){
            errors += "Level: Expected: " + std::to_string(m_expected_level) + ", received: " + std::to_string(level) + "\n";
        }

        return errors.empty() ? UnitTestResult(true) : UnitTestResult(std::move(errors));
    }

private:
    std::string m_image;
    int m_expected_dex;
    int m_expected_ot_id;
    int m_expected_level;
};

class Test_SummaryReader_Text : public UnitTest{
public:
    Test_SummaryReader_Text(const std::string& image, std::string expected_nature, std::string expected_ability, Language language)
        : UnitTest("PokemonHome::SummaryReader_Text - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected_nature(expected_nature)
        , m_expected_ability(expected_ability)
        , m_language(language)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        SummaryReader reader;

        std::string nature = reader.read_nature(logger, m_language, image);
        std::string ability = reader.read_ability(logger, m_language, image);
        std::string errors;
        if (nature != m_expected_nature){
            errors += "Nature: Expected: " + m_expected_nature + ", received: " + nature + "\n";
        }
        if (ability != m_expected_ability){
            errors += "Ability: Expected: " + m_expected_ability + ", received: " + ability + "\n";
        }

        return errors.empty() ? UnitTestResult(true) : UnitTestResult(std::move(errors));
    }

private:
    std::string m_image;
    std::string m_expected_nature;
    std::string m_expected_ability;
    Language m_language;
};

class Test_SummaryReader_OtName : public UnitTest{
public:
    Test_SummaryReader_OtName(const std::string& image, std::string expected_ot_name, Language language)
        : UnitTest("PokemonHome::SummaryReader_OtName - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected_ot_name(expected_ot_name)
        , m_language(language)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        SummaryReader reader;
        std::string ot_name = reader.read_original_trainer_name(m_language, image);
        if (ot_name == m_expected_ot_name){
            return true;
        }

        return "Original Trainer name: Expected: " + m_expected_ot_name + ", received: " + ot_name;
    }

private:
    std::string m_image;
    std::string m_expected_ot_name;
    Language m_language;
};

class Test_SummaryReader_LanguageOfOrigin : public UnitTest{
public:
    Test_SummaryReader_LanguageOfOrigin(const std::string& image, std::string expected)
        : UnitTest("PokemonHome::SummaryReader_LanguageOfOrigin - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        std::string language = SummaryReader().read_language_of_origin(image);
        if (language == m_expected){
            return true;
        }

        return "Expected: " + m_expected + ", received: " + language;
    }

private:
    std::string m_image;
    std::string m_expected;
};

void add_tests_SummaryReader(UnitTestDatabase& database){
    //Numbers
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/annihilape_Regular.png", 979, 493124, 75);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/bidoof_Regular.png", 399, 4610, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", 1, 493001, 6);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", 1, 493001, 4);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", 1, 725174, 51);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/capskid_Regular.png", 951, 493124, 15);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/castform_Regular.png", 351, 248782, 13);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/cyclizar_Regular.png", 967, 254804, 45);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", 982, 911993, 45);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", 982, 787315, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/enamorus_Shiny.png", 905, 250128, 50);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", 999, 493124, 35);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/glimmet_Regular.png", 969, 254804, 35);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gogoat_Regular.png", 673, 57594, 38);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", 984, 402737, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/hatterne_Regular.png", 858, 302862, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/houndstone_Regular.png", 972, 254804, 45);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ironBunde_Regular.png", 991, 92396, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", 991, 493124, 57);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", 993, 493124, 58);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ironThorns_Regular.png", 995, 493124, 57);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", 941, 254804, 45);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/kingler_Shiny.png", 99, 840209, 40);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/komala_Regular.png", 775, 254804, 45);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/krabby_Shiny.png", 98, 840209, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/machamp_Regular.png", 68, 639783, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/pancham_Shiny.png", 674, 225962, 29);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rapidash_Regular.png", 78, 144934, 37);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rellor_Regular.png", 953, 493124, 26);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/riolu_Regular.png", 447, 348226, 25);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", 722, 764041, 31);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/scovillain_Regular.png", 952, 493124, 41);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", 988, 402737, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/squirtle_Shiny.png", 7, 700052, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", 786, 181130, 62);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", 978, 493124, 52);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/teddiursa_Regular.png", 216, 333685, 29);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/terapagos_regular.png", 1024, 493124, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/vulpix_Regular.png", 37, 226403, 11);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/vulpix_Shiny.png", 37, 225962, 14);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/wartortle_Regular.png", 8, 379916, 15);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/wurmple_Regular.png", 265, 843926, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", 14, 40419, 8);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", 14, 40419, 8);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", 83, 801876, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", 68, 611957, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", 67, 611957, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", 66, 611957, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", 81, 611957, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", 82, 611957, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", 79, 801876, 48);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", 359, 330279, 63);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", 846, 999056, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", 482, 330279, 70);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", 625, 330279, 48);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", 406, 999056, 9);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", 427, 999056, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", 659, 260338, 8);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", 794, 330279, 70);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", 898, 999056, 80);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", 851, 330279, 29);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", 737, 431290, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", 4, 768144, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", 420, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", 546, 105260, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", 546, 105260, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", 845, 999056, 40);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", 104, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", 225, 330279, 40);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", 882, 299694, 10);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", 885, 894064, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", 356, 999056, 39);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", 309, 330279, 22);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", 309, 330279, 22);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", 890, 221118, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", 890, 999056, 62);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", 597, 972741, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", 661, 186292, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", 92, 999056, 27);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", 443, 136887, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", 443, 136887, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", 623, 330279, 38);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", 623, 330279, 38);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", 706, 847242, 51);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", 810, 209651, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", 533, 999056, 39);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", 130, 999056, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", 857, 999056, 38);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", 857, 999056, 34);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", 93, 999056, 34);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", 679, 717895, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", 593, 999056, 41);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", 647, 250212, 50);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", 264, 330279, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", 725, 395385, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", 725, 395385, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", 67, 999056, 30);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", 67, 999056, 29);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", 66, 999056, 14);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", 801, 768144, 50);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", 129, 773808, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", 809, 808809, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", 809, 808809, 100);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", 52, 494288, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", 52, 999056, 23);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", 11, 113125, 25);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", 868, 558526, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", 439, 330279, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", 258, 228890, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", 34, 330279, 57);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", 29, 910654, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", 32, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", 291, 999056, 21);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", 714, 999056, 23);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", 826, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", 675, 330279, 32);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", 25, 200212, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", 61, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", 710, 999056, 14);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", 509, 999056, 10);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", 280, 237114, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", 78, 330279, 41);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", 223, 999056, 42);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", 112, 999056, 41);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", 821, 330279, 6);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", 479, 768144, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", 844, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", 28, 32955, 47);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", 813, 209651, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", 537, 330279, 61);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", 319, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", 422, 25232, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", 756, 330279, 39);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", 850, 330279, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", 850, 330279, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", 850, 624848, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", 819, 330279, 24);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", 79, 330279, 16);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", 459, 999056, 26);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", 816, 209651, 5);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", 618, 330279, 25);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", 618, 330279, 25);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", 787, 330279, 70);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", 786, 330279, 70);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", 532, 999056, 13);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", 532, 330279, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", 532, 999056, 13);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", 532, 330279, 20);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", 532, 999056, 13);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", 748, 330279, 55);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", 849, 999056, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", 520, 330279, 22);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", 535, 999056, 11);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", 583, 999056, 38);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", 583, 999056, 38);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", 37, 330279, 24);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", 37, 999056, 24);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", 746, 330279, 22);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", 202, 330279, 60);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", 360, 330279, 1);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", 893, 201225, 70);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", 893, 210915, 70);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", 263, 330279, 7);
    database.add<Test_SummaryReader_Numbers>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", 263, 999056, 20);
    
    //Text
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/annihilape_Regular.png", "Hardy", "Inner Focus", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bidoof_Regular.png", "Lonely", "Simple", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", "Relaxed", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", "Lonely", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", "Sassy", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/capskid_Regular.png", "Sassy", "Insomnia", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/castform_Regular.png", "Modest", "Forecast", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cyclizar_Regular.png", "Impish", "Shed Skin", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", "Calm", "Rattled", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", "Relaxed", "Serene Grace", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/enamorus_Shiny.png", "Naive", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", "Quiet", "Rattled", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/glimmet_Regular.png", "Naive", "Toxic Debris", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gogoat_Regular.png", "Quirky", "Sap Sipper", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", "Adamant", "Protosynthesis", Language::ChineseSimplified);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/hatterne_Regular.png", "Quiet", "Healer", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/houndstone_Regular.png", "Bold", "Sand Rush", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironBunde_Regular.png", "Careful", "Quark Drive", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", "Calm", "Quark Drive", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", "Docile", "Quark Drive", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironThorns_Regular.png", "Careful", "Quark Drive", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", "Impish", "Volt Absorb", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/kingler_Shiny.png", "Lonely", "Sheer Force", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/komala_Regular.png", "Gentle", "Comatose", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/krabby_Shiny.png", "Hasty", "Hyper Cutter", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/machamp_Regular.png", "Bold", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/pancham_Shiny.png", "Naive", "Iron Fist", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rapidash_Regular.png", "Calm", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rellor_Regular.png", "Hasty", "Compound Eyes", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/riolu_Regular.png", "Rash", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", "Rash", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/scovillain_Regular.png", "Timid", "Insomnia", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", "Adamant", "Protosynthesis", Language::ChineseSimplified);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/squirtle_Shiny.png", "Quiet", "Torrent", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", "Sassy", "Psychic Surge", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", "Rash", "Commander", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/teddiursa_Regular.png", "Mild", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/terapagos_regular.png", "Hardy", "Tera Shift", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vulpix_Regular.png", "Naughty", "Flash Fire", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vulpix_Shiny.png", "Jolly", "Flash Fire", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wartortle_Regular.png", "Brave", "Torrent", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wurmple_Regular.png", "Sassy", "Shield Dust", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", "Gentle", "Shed Skin", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", "Gentle", "Shed Skin", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", "Jolly", "Steadfast", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", "Mild", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", "Hardy", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", "Hasty", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", "Hardy", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", "Timid", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", "Sassy", "Regenerator", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", "Calm", "Super Luck", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", "Sassy", "Swift Swim", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", "Calm", "Levitate", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", "Gentle", "Pressure", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", "Careful", "Natural Cure", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", "Mild", "Run Away", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", "Lax", "Pickup", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", "Docile", "Beast Boost", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", "Impish", "Unnerve", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", "Bashful", "White Smoke", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", "Adamant", "Battery", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", "Timid", "Solar Power", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", "Bashful", "Chlorophyll", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", "Modest", "Infiltrator", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", "Modest", "Infiltrator", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", "Hasty", "Gulp Missile", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", "Modest", "Rock Head", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", "Naughty", "Insomnia", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", "Naive", "Water Absorb", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", "Jolly", "Clear Body", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", "Careful", "Pressure", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", "Lonely", "Lightning Rod", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", "Lonely", "Lightning Rod", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", "Timid", "Pressure", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", "Jolly", "Pressure", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", "Brave", "Iron Barbs", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", "Bold", "Big Pecks", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", "Quirky", "Levitate", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", "Hasty", "Sand Veil", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", "Hasty", "Sand Veil", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", "Rash", "Iron Fist", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", "Rash", "Iron Fist", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", "Modest", "Hydration", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", "Adamant", "Grassy Surge", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", "Modest", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", "Serious", "Intimidate", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", "Hardy", "Anticipation", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", "Mild", "Anticipation", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", "Calm", "Levitate", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", "Adamant", "No Guard", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", "Modest", "Water Absorb", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", "Modest", "Justified", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", "Bold", "Pickup", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", "Adamant", "Blaze", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", "Adamant", "Blaze", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", "Bold", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", "Jolly", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", "Sassy", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", "Mild", "Soul-Heart", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", "Gentle", "Swift Swim", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", "Brave", "Iron Fist", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", "Brave", "Iron Fist", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", "Mild", "Rattled", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", "Rash", "Tough Claws", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", "Impish", "Shed Skin", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", "Calm", "Aroma Veil", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", "Naive", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", "Calm", "Damp", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", "Timid", "Sheer Force", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", "Lax", "Rivalry", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", "Mild", "Poison Point", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", "Naughty", "Speed Boost", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", "Quiet", "Frisk", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", "Bashful", "Telepathy", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", "Bold", "Mold Breaker", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", "Hardy", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", "Mild", "Damp", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", "Serious", "Frisk", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", "Lonely", "Limber", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", "Timid", "Trace", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", "Adamant", "Pastel Veil", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", "Rash", "Sniper", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", "Calm", "Rock Head", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", "Timid", "Unnerve", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", "Timid", "Levitate", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", "Lonely", "Shed Skin", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", "Naughty", "Sand Veil", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", "Jolly", "Libero", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", "Modest", "Poison Touch", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", "Quirky", "Rough Skin", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", "Relaxed", "Storm Drain", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", "Jolly", "Effect Spore", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", "Naughty", "Flash Fire", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", "Docile", "Flash Fire", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", "Quiet", "Flash Fire", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", "Bold", "Cheek Pouch", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", "Bold", "Own Tempo", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", "Bashful", "Snow Warning", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", "Modest", "Sniper", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", "Modest", "Mimicry", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", "Modest", "Mimicry", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", "Brave", "Grassy Surge", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", "Hasty", "Psychic Surge", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", "Mild", "Sheer Force", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", "Hardy", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", "Mild", "Sheer Force", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", "Hardy", "Guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", "Mild", "Sheer Force", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", "Gentle", "Regenerator", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", "Sassy", "Plus", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", "Docile", "Super Luck", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", "Brave", "Swift Swim", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", "Lonely", "Ice Body", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", "Lonely", "Ice Body", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", "Rash", "Flash Fire", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", "Lonely", "Flash Fire", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", "Hasty", "Schooling", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", "Lax", "Shadow Tag", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", "Relaxed", "Shadow Tag", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", "Adamant", "Leaf Guard", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", "Adamant", "Leaf Guard", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", "Adamant", "Pickup", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", "Lonely", "Gluttony", Language::English);

    //OT Name
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/annihilape_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/bidoof_Regular.png", "h1karu", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", "zazuba", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/capskid_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/castform_Regular.png", "kr0n0s", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/cyclizar_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", "bj0rn", Language::German);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", "mythra", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/enamorus_Shiny.png", "h0me", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/glimmet_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gogoat_Regular.png", "このは", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", "yam1", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/hatterne_Regular.png", "hunter", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/houndstone_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ironBunde_Regular.png", "t0r1", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ironThorns_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/kingler_Shiny.png", "maarxx1e", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/komala_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/krabby_Shiny.png", "maarxx1e", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/machamp_Regular.png", "g10r1a", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/pancham_Shiny.png", "m00n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rapidash_Regular.png", "emmy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rellor_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/riolu_Regular.png", "b0nd", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", "akar1", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/scovillain_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", "yam1", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/squirtle_Shiny.png", "puppycat1012", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", "aka1a", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/teddiursa_Regular.png", "コウ", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/terapagos_regular.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/vulpix_Regular.png", "bj0rn", Language::German);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/vulpix_Shiny.png", "m00n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/wartortle_Regular.png", "ru", Language::German);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/wurmple_Regular.png", "r0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", "da1t0n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", "1javy", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", "beg0", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", "dak0ta", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", "zazuba", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", "いすも", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", "いすも", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", "magg1e", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", "しよう", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", "ga1ar", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", "とと", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", "har00n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", "0", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", "0", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", "bagg1ns", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", "zazuba2", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", "かなて", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", "h0me", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", "1e1nad", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", "1e1nad", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", "zazuba", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", "dark", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", "h0me", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", "h0me", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", "fen1xz", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", "taka0", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", "agu", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", "cha0s", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", "ユウリ", Language::Japanese);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", "h0me", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", "우리", Language::Korean);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", "zazuba", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", "raym0nd", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", "zazuba2", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", "2de", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", "ka1t0", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", "zazuba2", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", "jung1e", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", "jung1e", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", "kev1n", Language::English);
    database.add<Test_SummaryReader_OtName>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", "kev1n", Language::English);
    
    // Language of Origin
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/annihilape_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/bidoof_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/capskid_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/castform_Regular.png", "ITA");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/cyclizar_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", "DEU");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/enamorus_Shiny.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/glimmet_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gogoat_Regular.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", "CHS");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/hatterne_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/houndstone_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ironBunde_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ironThorns_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/kingler_Shiny.png", "ITA");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/komala_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/krabby_Shiny.png", "ITA");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/machamp_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/pancham_Shiny.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rapidash_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rellor_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/riolu_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/scovillain_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", "CHS");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/squirtle_Shiny.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/teddiursa_Regular.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/terapagos_regular.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/vulpix_Regular.png", "DEU");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/vulpix_Shiny.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/wartortle_Regular.png", "DEU");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/wurmple_Regular.png", "FRA");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", "ES-ES");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", "ES-ES");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", "ES-ES");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", "ES-ES");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", "KOR");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", "JPN");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", "ITA");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", "ENG");
    database.add<Test_SummaryReader_LanguageOfOrigin>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", "ENG");

}
    

}
}
}
