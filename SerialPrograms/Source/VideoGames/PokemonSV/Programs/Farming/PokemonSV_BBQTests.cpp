/*  Pokemon SV BBQ Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "VideoGames/PokemonSV/Inference/PokemonSV_BlueberryPointsReader.h"
#include "VideoGames/PokemonSV/Inference/PokemonSV_BlueberryQuestDetector.h"
#include "VideoGames/PokemonSV/Options/PokemonSV_BBQuests.h"
#include "PokemonSV_BlueberryQuests.h"
#include "PokemonSV_BBQTests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

class Test_BBQuestsSlugs : public UnitTest{
public:
    Test_BBQuestsSlugs()
        : UnitTest("PokemonSV::BBQuests - slug round trip")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        for (int c = 0; c < (int)BBQuests::UnableToDetect; c++){
            const BBQuests quest = (BBQuests)c;
            const std::string& slug = BBQuests_enum_to_string(quest);
            if (slug.empty()){
                return "Quest " + std::to_string(c) + " has no slug.";
            }
            if (BBQuests_string_to_enum(slug) != quest){
                return "Slug " + slug + " does not map back to quest " + std::to_string(c) + ".";
            }
        }
        if (!BBQuests_enum_to_string(BBQuests::UnableToDetect).empty()){
            return "UnableToDetect must map to the empty slug.";
        }
        if (BBQuests_string_to_enum("not-a-quest") != BBQuests::UnableToDetect){
            return "An unrecognized slug must map to UnableToDetect.";
        }
        return true;
    }
};

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

struct TeraEncounterScenario{
    std::string name;
    bool shiny = false;
    int encounter_watcher_result;       //  0: battle menu, 1: audio, -1: route finished first
    bool battle_menu_detected = false;  //  The route's own wait saw the battle menu.
    TeraEncounterAction expected;
};

class Test_TeraEncounterAction : public UnitTest{
public:
    Test_TeraEncounterAction(TeraEncounterScenario scenario)
        : UnitTest("PokemonSV::TeraEncounterAction - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const TeraEncounterAction actual = tera_encounter_action(
            m_scenario.shiny, m_scenario.encounter_watcher_result, m_scenario.battle_menu_detected
        );
        if (actual != m_scenario.expected){
            return "Expected action " + std::to_string((int)m_scenario.expected)
                + ", got " + std::to_string((int)actual) + ".";
        }
        return true;
    }

private:
    const TeraEncounterScenario m_scenario;
};

//  One quest slot on a full frame of the quest panel.
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

static void add_tests_TeraEncounterAction(UnitTestDatabase& database){
    using enum TeraEncounterAction;
    const std::vector<TeraEncounterScenario> scenarios{
        {.name = "timeout recovers without battle", .encounter_watcher_result = -1, .expected = Recover},
        {.name = "audio alone is not entry", .encounter_watcher_result = 1, .expected = Recover},
        {.name = "unknown callback is not entry", .encounter_watcher_result = 2, .expected = Recover},
        {.name = "encounter watcher menu battles", .encounter_watcher_result = 0, .expected = Battle},
        {.name = "route wait menu battles", .encounter_watcher_result = -1, .battle_menu_detected = true, .expected = Battle},
        {.name = "audio then route wait menu battles", .encounter_watcher_result = 1, .battle_menu_detected = true, .expected = Battle},
        {.name = "both menus battle", .encounter_watcher_result = 0, .battle_menu_detected = true, .expected = Battle},
        {.name = "shiny stops without entry", .shiny = true, .encounter_watcher_result = -1, .expected = StopForShiny},
        {.name = "shiny stops before battle", .shiny = true, .encounter_watcher_result = 0, .battle_menu_detected = true, .expected = StopForShiny},
    };
    for (const TeraEncounterScenario& scenario : scenarios){
        database.add<Test_TeraEncounterAction>(scenario);
    }
}


static void add_tests_BlueberryQuestDetector(UnitTestDatabase& database){
    using enum BlueberryQuestDetector::QuestPosition;
    //  The second frame is the first after rerolling the first quest. Singleplayer
    //  without a red quest shows 3 quests, so the fourth slot reads as nothing.
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


void add_tests_BBQ(UnitTestDatabase& database){
    database.add<Test_BBQuestsSlugs>();

    // Full-frame references, in the CommandLineTests repo: they exercise the production
    // crop, filter and OCR together.
    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6750.png", 6750, 1920, 1080);
    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6750.png", 6750, 1280, 720);

    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6740.png", 6740, 1920, 1080);
    database.add<Test_BlueberryPoints>("PokemonSV/BlueberryPoints/6740.png", 6740, 1280, 720);

    add_tests_TeraEncounterAction(database);
    add_tests_BlueberryQuestDetector(database);
}



}
}
}
