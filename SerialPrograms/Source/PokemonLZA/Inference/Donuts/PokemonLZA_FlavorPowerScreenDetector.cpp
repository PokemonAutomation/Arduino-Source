/*  Flavor Power Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Filesystem/Filesystem.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonTools/Images/SolidColorTest.h"
//#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonLZA_FlavorPowerDetector.h"
#include "PokemonLZA_FlavorPowerScreenDetector.h"
#include "Tests/TestUtils.h"

#include <fstream>
#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


FlavorPowerScreenDetector::FlavorPowerScreenDetector()
    : m_left_white_area(0.044, 0.238, 0.021, 0.143)
    , m_bottom_white_area(0.105, 0.459, 0.090, 0.036)
    , m_donut_area(0.077, 0.182, 0.149, 0.252)
{}

void FlavorPowerScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_WHITE, m_left_white_area);
    items.add(COLOR_WHITE, m_bottom_white_area);
}

bool FlavorPowerScreenDetector::detect(const ImageViewRGB32& screen){
    const ImageStats stats2 = image_stats(extract_box_reference(screen, m_left_white_area));
    if (!is_solid(stats2, {0.341, 0.340, 0.319})){
        return false;
    }

    const ImageStats stats3 = image_stats(extract_box_reference(screen, m_bottom_white_area));
    if (!is_solid(stats3, {0.341, 0.340, 0.319})){
        return false;
    }

    const ImageStats stats4 = image_stats(extract_box_reference(screen, m_donut_area));
    if (stats4.stddev.sum() < 50.0){
        return false;
    }

    return true;
}


class Test_FlavorPowerScreenDetector : public UnitTest{
public:
    Test_FlavorPowerScreenDetector(const std::string& image)
        : UnitTest("PokemonPLZA::FlavorPowerScreenDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        Filesystem::Path file_path(m_image);
        Filesystem::Path parent_dir = file_path.parent_path();
        std::string base_name = file_path.stem().string();

        const std::vector<std::string> words = parse_words(base_name);
        if (words.empty()){
            return "Error: not enough number of words in the filename.";
        }

        std::string code = words.back();
        if (code == "chiSim"){
            code = "chi_sim";
        }else if (code == "chiTra"){
            code = "chi_tra";
        }
        Language language = language_code_to_enum(code);
        if (language == Language::None || language == Language::EndOfList){
            return "Error: invalid language word in filename.";
        }

        ImageRGB32 image(m_image);
        FlavorPowerScreenDetector screen_detector;
        if (!screen_detector.detect(image)){
            return "Error: FlavorPowerScreenDetector did not detect flavor power screen.";
        }

        if (STATIC_GLOBALS.GENERATE_TEST_GOLDEN_FILES){
            Filesystem::Path target_powers_path = parent_dir / ("_" + base_name + ".txt");
            std::ofstream output_file(target_powers_path.stdpath());
            if (!output_file.is_open()){
                return "Error: cannot open output file " + target_powers_path.string() + " for writing";
            }

            for (int i = 0; i < 3; i++){
                FlavorPowerDetector power_detector(global_logger_command_line(), COLOR_RED, language, i);
                std::string power_slug = power_detector.detect_power(image);
                output_file << (power_slug.empty() ? "empty" : power_slug) << std::endl;
            }
            return true;
        }

        Filesystem::Path target_powers_path = parent_dir / ("_" + base_name + ".txt");
        std::vector<std::string> expected_powers;
        if (!load_slug_list(target_powers_path.string(), expected_powers)){
            return "Loading slug list " + target_powers_path.string() + " failed.";
        }
        if (expected_powers.size() != 3){
            return "Error: need to have exactly 3 power slots in " + target_powers_path.string();
        }

        for (int i = 0; i < 3; i++){
            FlavorPowerIconDetector power_icon_detector(global_logger_command_line(), i);
            FlavorPowerDetector power_detector(global_logger_command_line(), COLOR_RED, language, i);

            std::string detected_power_slug = power_detector.detect_power(image);
            int detected_power_level = power_icon_detector.detect(image);
            const std::string& expected_power = expected_powers[i];

            if (expected_power == "empty"){
                if (!detected_power_slug.empty()){
                    return "Error: expected empty power slot but OCR detected a power.";
                }
                if (detected_power_level > 0){
                    return "Error: expected empty power slot but icon detector detected a level.";
                }
                continue;
            }

            TEST_RESULT_COMPONENT_EQUAL_STR(detected_power_slug, expected_power, "power slug for slot " + std::to_string(i));
            if (detected_power_slug.empty()){
                return "Error: OCR detected empty but expected a power.";
            }
            int expected_level = detected_power_slug.back() - '1' + 1;
            if (detected_power_level != expected_level){
                std::ostringstream ss;
                ss << "Error: Slot " << i << " - OCR reads power " << detected_power_slug
                   << " but power icon detector gets power level " << detected_power_level;
                return ss.str();
            }
        }

        return true;
    }

private:
    std::string m_image;
};


void add_tests_FlavorPowerScreenDetector(UnitTestDatabase& database){
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/gin_01_eng.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/gin_02_eng.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jimin_01_kor.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jimin_02_kor.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_01_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_02_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_03_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_04_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_05_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_06_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_07_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_08_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/jojoaman_09_chiTra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/kichi_01_eng.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/kuro_01_normal_eng.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/kuro_02_dark_eng.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/ougi_01_chiSim.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/ougi_02_chiSim.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/pif_01_fra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/pif_02_fra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/pif_03_fra.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/turboV_01_eng.jpg");
    database.add<Test_FlavorPowerScreenDetector>("PokemonLZA/FlavorPowerScreenDetector/turboV_02_eng.jpg");
}


}
}
}
