/*  Pokemon Home Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include <vector>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Strings/Unicode.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "PokemonHome_SummaryReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

namespace{

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
{}

void SummaryReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_national_dex_number_box);
    items.add(m_color, m_level_box);
    items.add(m_color, m_original_trainer_id_box);
    items.add(m_color, m_original_trainer_name_box);
    items.add(m_color, m_nature_box);
    items.add(m_color, m_ability_box);
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

std::string SummaryReader::read_nature(Language language, const ImageViewRGB32& screen) const{
    return read_text(language, screen, m_nature_box, gray_text_filters());
}

std::string SummaryReader::read_ability(Language language, const ImageViewRGB32& screen) const{
    return read_text(language, screen, m_ability_box, gray_text_filters());
}

// Due to the position of the level changing slightly depending on digits and language. The box contains the "Lv" text and the level number.
int SummaryReader::read_level(Logger& logger, const ImageViewRGB32& screen) const{
    try{
        std::string text = read_text(Language::English, screen, m_level_box, gray_text_filters());
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
        return reader.read_national_dex(logger, image) == m_expected_dex
            && reader.read_original_trainer_id(logger, image) == m_expected_ot_id
            && reader.read_level(logger, image) == m_expected_level;
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
        return reader.read_nature(m_language, image) == m_expected_nature
            && reader.read_ability(m_language, image) == m_expected_ability;
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
        return reader.read_original_trainer_name(m_language, image) == m_expected_ot_name;
    }

private:
    std::string m_image;
    std::string m_expected_ot_name;
    Language m_language;
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
    //Text
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/annihilape_Regular.png", "hardy", "1nnerf0cus", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bidoof_Regular.png", "10ne1y", "s1mp1e", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", "re1axed", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", "10ne1y", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", "sassy", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/capskid_Regular.png", "sassy", "1ns0mn1a", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/castform_Regular.png", "m0dest", "f0recast", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/cyclizar_Regular.png", "1mp1sh", "shedsk1n", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", "st111", "hasenfub", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", "re1axed", "serenegrace", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/enamorus_Shiny.png", "na1ve", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", "qu1et", "ratt1ed", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/glimmet_Regular.png", "na1ve", "t0x1cdebr1s", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/gogoat_Regular.png", "qu1rky", "saps1pper", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", "固执", "古代活性", Language::ChineseSimplified);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/hatterne_Regular.png", "qu1et", "hea1er", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/houndstone_Regular.png", "b01d", "sandrush", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironBunde_Regular.png", "carefu1", "quarkdr1ve", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", "ca1m", "quarkdr1ve", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", "d0c11e", "quarkdr1ve", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/ironThorns_Regular.png", "carefu1", "quarkdr1ve", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", "1mp1sh", "v01tabs0rb", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/kingler_Shiny.png", "10ne1y", "sheerf0rce", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/komala_Regular.png", "gent1e", "c0mat0se", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/krabby_Shiny.png", "hasty", "hypercutter", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/machamp_Regular.png", "b01d", "guts", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/pancham_Shiny.png", "na1ve", "1r0nf1st", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rapidash_Regular.png", "ca1m", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rellor_Regular.png", "hasty", "c0mp0undeyes", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/riolu_Regular.png", "rash", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", "rash", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/scovillain_Regular.png", "t1m1d", "1ns0mn1a", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", "固执", "古代活性", Language::ChineseSimplified);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/squirtle_Shiny.png", "ruh1g", "sturzbach", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", "sassy", "psych1csurge", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", "rash", "c0mmander", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/teddiursa_Regular.png", "m11d", "", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/terapagos_regular.png", "hardy", "terash1ft", Language::English);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vulpix_Regular.png", "frech", "feuerfanger", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/vulpix_Shiny.png", "fr0h", "feuerfanger", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wartortle_Regular.png", "mut1g", "sturzbach", Language::German);
    database.add<Test_SummaryReader_Text>("PokemonHome/SummaryScreen/wurmple_Regular.png", "sassy", "sh1e1ddust", Language::English);
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
}
    

}
}
}
