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
#include "Programs/Farming/PokemonSV_BlueberryQuests.h"
#include "PokemonSV_Tests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

namespace{

class NullLogger : public Logger{
public:
    virtual void log(const std::string&, Color) override{}
};

// The quests read from the panel, the options that matter, and the plan
// plan_quests() must make. The default exclusions table has one row:
// auto-10, skip.
struct BBQuestPlanScenario{
    std::string name;
    std::vector<BBQuests> quests;
    std::vector<std::pair<BBQuests, BBQAction>> extra_exclusions = {};
    uint8_t num_eggs = 1;
    uint8_t eggs_hatched = 0;
    BBQOption::OOEggs out_of_eggs = BBQOption::OOEggs::Reroll;
    std::vector<BBQuests> expected_to_do = {};
    std::vector<size_t> expected_reroll_slots = {};
    bool expected_reroll_all = false;
};

std::string quests_to_string(const std::vector<BBQuests>& quests){
    std::string ret;
    for (BBQuests quest : quests){
        ret += std::to_string((int)quest) + " ";
    }
    return ret;
}
std::string slots_to_string(const std::vector<size_t>& slots){
    std::string ret;
    for (size_t slot : slots){
        ret += std::to_string(slot) + " ";
    }
    return ret;
}

class Test_BBQuestPlan : public UnitTest{
public:
    Test_BBQuestPlan(BBQuestPlanScenario scenario)
        : UnitTest("PokemonSV::BBQuestPlan - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        BBQOption options;
        options.NUM_EGGS.set(m_scenario.num_eggs);
        options.OUT_OF_EGGS.set(m_scenario.out_of_eggs);
        for (const auto& [quest, action] : m_scenario.extra_exclusions){
            auto row = std::make_unique<BBQuestTableRow>(options.QUEST_EXCLUSIONS);
            row->quest.set(quest);
            row->action.set(action);
            options.QUEST_EXCLUSIONS.append_row(std::move(row));
        }

        NullLogger logger;
        const BBQuestPlan plan = plan_quests(logger, options, m_scenario.quests, m_scenario.eggs_hatched);
        if (plan.to_do != m_scenario.expected_to_do){
            return "Expected quests to do: " + quests_to_string(m_scenario.expected_to_do)
                + "got: " + quests_to_string(plan.to_do);
        }
        if (plan.reroll_slots != m_scenario.expected_reroll_slots){
            return "Expected reroll slots: " + slots_to_string(m_scenario.expected_reroll_slots)
                + "got: " + slots_to_string(plan.reroll_slots);
        }
        if (plan.reroll_all != m_scenario.expected_reroll_all){
            return std::string("Expected reroll_all = ") + (m_scenario.expected_reroll_all ? "true" : "false");
        }
        return true;
    }

private:
    const BBQuestPlanScenario m_scenario;
};

}

void add_tests_BBQuestPlan(UnitTestDatabase& database){
    using enum BBQuests;
    const BBQuestPlanScenario scenarios[] = {
        {
            .name = "default table skips auto-10",
            .quests = {make_tm, auto_10, sneak_up},
            .expected_to_do = {make_tm, sneak_up},
        },
        {
            .name = "quests not possible are left out",
            .quests = {pickup_10, make_tm, UnableToDetect},
            .expected_to_do = {make_tm},
        },
        {
            .name = "nothing doable rerolls all",
            .quests = {pickup_10, auto_10, catch_flying},
            .expected_reroll_all = true,
        },
        {
            // The reroll replaces a quest, so the next read has something new.
            .name = "a reroll row rerolls that slot",
            .quests = {make_tm, sneak_up, travel_500},
            .extra_exclusions = {{travel_500, BBQAction::reroll}},
            .expected_to_do = {make_tm, sneak_up},
            .expected_reroll_slots = {2},
        },
        {
            // A reroll is already a change, so nothing more is rerolled.
            .name = "a rerolled slot with nothing doable does not reroll all",
            .quests = {pickup_10, sneak_up, auto_10},
            .extra_exclusions = {{sneak_up, BBQAction::reroll}},
            .expected_reroll_slots = {1},
        },
        {
            // Each row for a quest applies, in table order.
            .name = "every table row for a quest applies",
            .quests = {travel_500, auto_10},
            .extra_exclusions = {
                {travel_500, BBQAction::run},
                {auto_10, BBQAction::run},
                {travel_500, BBQAction::run},
            },
            .expected_to_do = {travel_500, travel_500, auto_10},
        },
        {
            .name = "egg quest is done while eggs remain",
            .quests = {make_tm, hatch_egg, sneak_up},
            .expected_to_do = {make_tm, hatch_egg, sneak_up},
        },
        {
            // The egg quest is rerolled where it is on the panel, not always
            // in the top slot.
            .name = "out of eggs rerolls the egg quest slot",
            .quests = {make_tm, hatch_egg, sneak_up},
            .eggs_hatched = 1,
            .expected_to_do = {make_tm, sneak_up},
            .expected_reroll_slots = {1},
        },
        {
            .name = "out of eggs with keep going leaves the egg quest",
            .quests = {make_tm, hatch_egg, sneak_up},
            .eggs_hatched = 1,
            .out_of_eggs = BBQOption::OOEggs::KeepGoing,
            .expected_to_do = {make_tm, sneak_up},
        },
    };
    for (const BBQuestPlanScenario& scenario : scenarios){
        database.add<Test_BBQuestPlan>(scenario);
    }
}



void add_tests(UnitTestDatabase& database){
    add_tests_BBQuestPlan(database);
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
