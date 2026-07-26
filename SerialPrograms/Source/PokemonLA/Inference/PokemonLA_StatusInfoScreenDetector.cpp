/*  Status Info Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Tests/TestUtils.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_CommonColorCheck.h"
#include "PokemonLA_StatusInfoScreenDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



PokemonDetails read_status_info(
    Logger& logger, VideoOverlay& overlay,
    const ImageViewRGB32& frame,
    Language language
){
    OverlayBoxScope shiny_box(overlay, {0.726, 0.133, 0.015, 0.023}, COLOR_BLUE);
    OverlayBoxScope alpha_box(overlay, {0.750, 0.133, 0.015, 0.023}, COLOR_RED);
    OverlayBoxScope gender_box(overlay, {0.777, 0.138, 0.001, 0.015}, COLOR_PURPLE);
    OverlayBoxScope name_box(overlay, {0.525, 0.130, 0.100, 0.038}, COLOR_BLACK);

    PokemonDetails ret;

    {
        const ImageStats shiny_box_stats = image_stats(extract_box_reference(frame, shiny_box));
        // std::cout << "ImageStats " << shiny_box_stats.average << "  " << shiny_box_stats.stddev << std::endl;
        const auto& stddev = shiny_box_stats.stddev;
        const double max_stddev = std::max(std::max(stddev.r, stddev.g), stddev.b);
        if(!is_solid(shiny_box_stats, {0.333333, 0.333333, 0.333333}, 0.2, 15) || max_stddev > 8.){
            ret.is_shiny = true;
            logger.log("Detected Shiny!", COLOR_BLUE);
        }
    }

    const ImageStats alpha_stats = image_stats(extract_box_reference(frame, alpha_box));
    if (alpha_stats.stddev.sum() > 80 &&
        alpha_stats.average.r > alpha_stats.average.g + 30 &&
        alpha_stats.average.r > alpha_stats.average.b + 30
    ){
        ret.is_alpha = true;
        logger.log("Detected Alpha!", COLOR_BLUE);
    }

    const ImageStats gender_stats = image_stats(extract_box_reference(frame, gender_box));
//    cout << gender_stats.average << gender_stats.stddev << endl;
    if (is_solid(gender_stats, {0.333333, 0.333333, 0.333333}, 0.1, 10)){
        ret.gender = Gender::Genderless;
        logger.log("Gender: Genderless");
    }else if (gender_stats.average.b > gender_stats.average.g + 30 && gender_stats.average.b > gender_stats.average.r + 30){
        ret.gender = Gender::Male;
        logger.log("Gender: Male");
    }else if (gender_stats.average.r > gender_stats.average.g + 30 && gender_stats.average.r > gender_stats.average.b + 30){
        ret.gender = Gender::Female;
        logger.log("Gender: Female");
    }else{
        logger.log("Gender: Unable to detect", COLOR_RED);
    }

    if (language == Language::None){
        return ret;
    }

    ImageViewRGB32 image = extract_box_reference(frame, name_box);

    OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
        logger, language, image,
        OCR::BLACK_TEXT_FILTERS()
    );

    for (auto& item : result.results){
        ret.name_candidates.insert(std::move(item.second.token));
    }

    return ret;
}







std::string read_pokemon_info_from_words(const std::vector<std::string>& keywords, Language& language, PokemonDetails& details){
    // the last five keywords should be: <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>

    if (keywords.size() < 5){
        std::stringstream ss;
        ss << "Error: not enough number of keywords in the filename to generate PokemonDetails. Found only " << keywords.size() << "." << endl;
        return ss.str();
    }

    language = language_code_to_enum(keywords[keywords.size()-5]);
    if (language == Language::None || language == Language::EndOfList){
        std::stringstream ss;
        ss << "Error: language keyword " << keywords[keywords.size()-5] << " is wrong." << endl;
        return ss.str();
    }

    const std::string& pokemon_slug = keywords[keywords.size()-4];
    details.name_candidates.insert(pokemon_slug);

    const std::string& shiny_word = keywords[keywords.size()-3];
    if (shiny_word == "Shiny"){
        details.is_shiny = true;
    }else if (shiny_word == "NotShiny"){
        details.is_shiny = false;
    }else{
        std::stringstream ss;
        ss << "Error: shiny keyword " << shiny_word << " is wrong. Must be \"Shiny\" or \"NotShiny\"." << endl;
        return ss.str();
    }

    const std::string& alpha_word = keywords[keywords.size()-2];
    if (alpha_word == "Alpha"){
        details.is_alpha = true;
    }else if (alpha_word == "NotAlpha"){
        details.is_alpha = false;
    }else{
        std::stringstream ss;
        ss << "Error: alpha keyword " << alpha_word << " is wrong. Must be \"Alpha\" or \"NotAlpha\"." << endl;
        return ss.str();
    }

    const std::string& gender_word = keywords[keywords.size()-1];
    if (gender_word == "Male"){
        details.gender = Gender::Male;
    }else if (gender_word == "Female"){
        details.gender = Gender::Female;
    }else if (gender_word == "Genderless"){
        details.gender = Gender::Genderless;
    }else{
        std::stringstream ss;
        ss << "Error: gender keyword " << gender_word << " is wrong. Must be \"Male\", \"Female\" or \"Genderless\"." << endl;
        return ss.str();
    }

    return "";
}




std::string test_pokemon_details(const PokemonDetails& details, const PokemonDetails& target){
    TEST_RESULT_COMPONENT_EQUAL_STR(details.is_shiny, target.is_shiny, "shiny");
    TEST_RESULT_COMPONENT_EQUAL_STR(details.is_alpha, target.is_alpha, "alpha");
    TEST_RESULT_COMPONENT_EQUAL_WITH_PRINT_FUNC_STR(details.gender, target.gender, "gender", get_gender_str);

    const std::string& pokemon_slug = *target.name_candidates.begin();
    bool found_name = false;
    for (const auto& slg : details.name_candidates){
        if (slg == pokemon_slug){
            found_name = true;
            break;
        }
    }
    if (found_name == false){
        std::stringstream ss;
        ss << "Error: " << __func__ << " name result is ";
        for (const auto& slg : details.name_candidates){
            ss << slg << ", ";
        }
        ss << "but should be " << pokemon_slug << "." << endl;
        return ss.str();
    }

    return "";
}



class Test_StatusInfoScreenDetector : public UnitTest{
public:
    Test_StatusInfoScreenDetector(
        const std::string& image,
        std::vector<std::string> keywords
    )
        : UnitTest("PokemonLA::StatusInfoScreenDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_keywords(std::move(keywords))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        // the last five keywords should be: <language> <pokemon name slug> <Shiny/NotShiny> <Alpha/NotAlpha> <Male/Female/Genderless>
        Language language = Language::None;
        PokemonDetails target;

        std::string ret = read_pokemon_info_from_words(m_keywords, language, target);
        if (!ret.empty()){
            return ret;
        }

        DummyVideoOverlay overlay;
        ImageRGB32 image(m_image);
        const PokemonDetails details = read_status_info(logger, overlay, image, language);

        ret = test_pokemon_details(details, target);
        if (ret.empty()){
            return true;
        }
        return ret;
    };

private:
    std::string m_image;
    std::vector<std::string> m_keywords;
};



void add_tests_StatusInfoScreenDetector(UnitTestDatabase& database){
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/macOS_bright/eng_arceus_NotShiny_NotAlpha_Genderless.png",
        parse_words("eng_arceus_NotShiny_NotAlpha_Genderless")
    );
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/macOS_bright/eng_graveler_Shiny_Alpha_Male.png",
        parse_words("eng_graveler_Shiny_Alpha_Male")
    );
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/macOS_bright/eng_snorunt_NotShiny_NotAlpha_Female.png",
        parse_words("eng_snorunt_NotShiny_NotAlpha_Female")
    );

    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinElgato/eng_burmy_NotShiny_NotAlpha_Female.png",
        parse_words("eng_burmy_NotShiny_NotAlpha_Female")
    );
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinElgato/eng_burmy_NotShiny_NotAlpha_Male.png",
        parse_words("eng_burmy_NotShiny_NotAlpha_Male")
    );
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinElgato/eng_geodude_NotShiny_NotAlpha_Male.jpg",
        parse_words("eng_geodude_NotShiny_NotAlpha_Male")
    );

    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinMyPin/eng_gardevoir_Shiny_Alpha_Female.png",
        parse_words("eng_gardevoir_Shiny_Alpha_Female")
    );
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinMyPin/eng_geodude_NotShiny_NotAlpha_Male.png",
        parse_words("eng_geodude_NotShiny_NotAlpha_Male")
    );

    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinShadowCast/eng_gardevoir_Shiny_Alpha_Female.png",
        parse_words("eng_gardevoir_Shiny_Alpha_Female")
    );
    database.add<Test_StatusInfoScreenDetector>(
        "PokemonLA/StatusInfoScreenDetector/WinShadowCast/eng_geodude_NotShiny_NotAlpha_Male.png",
        parse_words("eng_geodude_NotShiny_NotAlpha_Male")
    );
}



}
}
}
