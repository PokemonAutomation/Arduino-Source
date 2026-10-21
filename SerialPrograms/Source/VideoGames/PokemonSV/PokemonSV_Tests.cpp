/*  Pokemon SV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Inference/PokemonSV_WhiteButtonDetector.h"
#include "Inference/Boxes/PokemonSV_BoxDetection.h"
#include "Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "Inference/Map/PokemonSV_MapDetector.h"
#include "Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "Inference/Tera/PokemonSV_TeraSilhouetteReader.h"
#include "Inference/Tera/PokemonSV_TeraTypeReader.h"
#include "Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "Inference/Map/PokemonSV_FastTravelDetector.h"
#include "Inference/Map/PokemonSV_MapMenuDetector.h"
#include "Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
#include "Inference/PokemonSV_ESPEmotionDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichPlateDetector.h"
#include "Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV_Tests.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Inference/PokemonSV_BlueberryPointsReader.h"
#include "Inference/PokemonSV_BlueberryQuestDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



namespace{

class Test_BlueberryPoints : public UnitTest{
public:
    Test_BlueberryPoints(const std::string& image, int expected, size_t width, size_t height)
        : UnitTest("PokemonSV::BlueberryPoints - " + image + " - " + std::to_string(width))
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected), m_width(width), m_height(height)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        ImageRGB32 reference(m_image);
        ImageRGB32 image = reference.scale_to(m_width, m_height);
        const int actual = read_BP_from_image(logger, image);
        if (actual != m_expected){
            return "Expected " + std::to_string(m_expected) + " BP, read " + std::to_string(actual);
        }
        return true;
    }

private:
    const std::string m_image;
    const int m_expected;
    const size_t m_width, m_height;
};

// One quest slot on a full frame of the quest panel.
struct BlueberryQuestScenario{
    std::string image;
    Language language;
    BlueberryQuestDetector::QuestPosition position;
    std::string expected_slug;
    size_t width;
    size_t height;
};

class Test_BlueberryQuestDetector : public UnitTest{
public:
    Test_BlueberryQuestDetector(BlueberryQuestScenario scenario)
        : UnitTest(
            "PokemonSV::BlueberryQuestDetector - " + scenario.image
            + " - slot " + std::to_string((int)scenario.position + 1)
            + " - " + std::to_string(scenario.width)
        )
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        ImageRGB32 reference(UNIT_TEST_RESOURCE_PATH() + m_scenario.image);
        ImageRGB32 image = reference.scale_to(m_scenario.width, m_scenario.height);
        BlueberryQuestDetector detector(logger, COLOR_RED, m_scenario.language, m_scenario.position);
        const std::string actual = detector.detect_quest(image);
        if (actual != m_scenario.expected_slug){
            return "Expected quest " + m_scenario.expected_slug + ", read \"" + actual + "\"";
        }
        return true;
    }

private:
    const BlueberryQuestScenario m_scenario;
};

}

void add_tests_BlueberryQuestDetector(UnitTestDatabase& database){
    using enum BlueberryQuestDetector::QuestPosition;
    // The second frame is the first after rerolling the first quest. Singleplayer
    // without a red quest shows 3 quests, so the fourth slot reads as nothing.
    const struct{
        std::string image;
        std::string slugs[4];
    } panels[] = {
        {"PokemonSV/BlueberryPoints/6750.png", {"catch-any", "sneak-up", "auto-10", ""}},
        {"PokemonSV/BlueberryPoints/6740.png", {"photo-swim", "sneak-up", "auto-10", ""}},
    };
    const BlueberryQuestDetector::QuestPosition positions[] = {FIRST, SECOND, THIRD, FOURTH};
    const std::pair<size_t, size_t> sizes[] = {{1920, 1080}, {1280, 720}};
    for (const auto& panel : panels){
        for (size_t slot = 0; slot < 4; slot++){
            for (const auto& [width, height] : sizes){
                database.add<Test_BlueberryQuestDetector>(BlueberryQuestScenario{
                    .image = panel.image,
                    .language = Language::ChineseTraditional,
                    .position = positions[slot],
                    .expected_slug = panel.slugs[slot],
                    .width = width,
                    .height = height,
                });
            }
        }
    }
}

void add_tests(UnitTestDatabase& database){
    // Full-frame references exercise the production crop, filter and OCR together.
    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6750.png", 6750, 1920, 1080);
    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6750.png", 6750, 1280, 720);

    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6740.png", 6740, 1920, 1080);
    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6740.png", 6740, 1280, 720);
    add_tests_BlueberryQuestDetector(database);

    add_tests_WhiteButtonDetector(database);
    add_tests_BoxInfoDetector(database);
    add_tests_BoxEggDetector(database);
    add_tests_DialogDetector(database);
    add_tests_ESPEmotionDetector(database);
    add_tests_FastTravelDetector(database);
    add_tests_MapDetector(database);
    add_tests_MapMenuDetector(database);
    add_tests_MapPokeCenterIconDetector(database);
    add_tests_OverworldDetector(database);
    add_tests_PicnicDetector(database);
    add_tests_SandwichIngredientDetector(database);
    add_tests_SandwichPlateDetector(database);
    add_tests_SandwichRecipeDetector(database);
    add_tests_SandwichHandDetector(database);
    add_tests_TeraCardDetector(database);
    add_tests_TeraSilhouetteReader(database);
    add_tests_TeraTypeReader(database);
    add_tests_NormalBattleMenus(database);
    add_tests_LetsGoKillDetector(database);
}



}
}
}
