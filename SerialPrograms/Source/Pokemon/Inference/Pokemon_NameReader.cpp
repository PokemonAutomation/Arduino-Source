/*  Pokemon Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Tests/ImageUnitTest.h"
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

class Test_PokemonNameReader : public ImageUnitTest{
public:
    Test_PokemonNameReader(
        const std::string& image,
        Language language,
        const std::string& expected
    )
        : ImageUnitTest("OCR::RawOCR - " + image, {image})
        , m_language(language)
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const ImageRGB32& image = images[0];

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
    Language m_language;
    std::string m_expected;
};


void add_tests_PokemonNameReader(UnitTestDatabase& database){
    // database.add<Test_PokemonNameReader>("OCR/PokemonNameOCR/clefairy-20210618-211817.png", Language::Korean, "clefairy");
    // database.add<Test_PokemonNameReader>("OCR/PokemonNameOCR/clefable-20210618-212311.png", Language::Korean, "clefable");
}


}
}

