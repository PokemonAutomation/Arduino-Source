/*  Pokemon Home Summary Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
    };
    return filters;
}

const std::vector<std::pair<uint32_t, uint32_t>>& gray_number_filters(){
    static const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0xff5c6460, 0xffe1e8e4},
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
        {true, 0xff5c6460, 0xffe1e8e4},
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
    , m_level_box(0.546, 0.099, 0.044, 0.041)
    , m_original_trainer_id_box(0.782, 0.719, 0.193, 0.046)
    , m_original_trainer_name_box(0.492, 0.719, 0.165, 0.049)
    , m_nature_box(0.157, 0.783, 0.212, 0.042)
    , m_ability_box(0.158, 0.838, 0.213, 0.042)
{}

void SummaryReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_national_dex_number_box, "national dex");
    items.add(m_color, m_level_box, "level");
    items.add(m_color, m_original_trainer_id_box, "original trainer id");
    items.add(m_color, m_original_trainer_name_box, "original trainer name");
    items.add(m_color, m_nature_box, "nature");
    items.add(m_color, m_ability_box, "ability");
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

int SummaryReader::read_level(Logger& logger, const ImageViewRGB32& screen) const{
    return read_number(logger, screen, m_level_box, gray_number_filters());
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

void add_tests_SummaryReader(UnitTestDatabase& database){}


}
}
}
