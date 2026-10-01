/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace Pokemon{


const PokemonNameReader& PokemonNameReader::instance(){
    static PokemonNameReader reader;
    return reader;
}


PokemonNameReader::PokemonNameReader()
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-", nullptr, false)
{}
PokemonNameReader::PokemonNameReader(const std::set<std::string>& subset)
    : LargeDictionaryMatcher("Pokemon/PokemonNameOCR/PokemonOCR-", &subset, false)
{}

OCR::StringMatchResult PokemonNameReader::read_substring(
    Logger& logger,
    Language language,
    const ImageViewRGB32& image,
    const std::vector<OCR::TextColorRange>& text_color_ranges,
    double min_text_ratio, double max_text_ratio,
    double max_log10p
) const{
    return match_substring_from_image_multifiltered(
        &logger, language, image, text_color_ranges,
        max_log10p, MAX_LOG10P_SPREAD, min_text_ratio, max_text_ratio
    );  
}

class Test_PokemonNameReader : public UnitTest{
public:
    Test_PokemonNameReader(
        const std::string& image,
        Language language,
        const std::string& expected
    )
        : UnitTest("OCR::RawOCR - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_language(language)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        OCR::StringMatchResult ocr_result = Pokemon::PokemonNameReader::instance().read_substring(
            logger, m_language, image, OCR::BLACK_OR_WHITE_TEXT_FILTERS()
        );

        std::multimap<double, OCR::StringMatchData> results;
        if (!ocr_result.results.empty()){
            for (const auto& result : ocr_result.results){
                results.emplace(result.first, result.second);
            }
        }
        std::string string_result = "";

        if (results.empty()){
            string_result = "";
        }else if (results.size() > 1){
            logger.log("Unable to read selected item. Ambiguous or multiple results.");
            return false;
        }else{
            string_result = results.begin()->second.token;
        }

        return string_result == m_expected;
    };

private:
    std::string m_image;
    Language m_language;
    std::string m_expected;
};


void add_tests_PokemonNameReader(UnitTestDatabase& database){
    // database.add<Test_PokemonNameReader>("OCR/PokemonNameOCR/clefairy-20210618-211817.png", Language::Korean, "clefairy");
    // database.add<Test_PokemonNameReader>("OCR/PokemonNameOCR/clefable-20210618-212311.png", Language::Korean, "clefable");
}


}
}

