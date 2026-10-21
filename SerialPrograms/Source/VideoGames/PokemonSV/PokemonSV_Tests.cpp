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
    std::vector<std::pair<BBQuests, BBQAction>> extra_exclusions;
    uint8_t num_eggs = 1;
    uint8_t eggs_hatched = 0;
    BBQOption::OOEggs out_of_eggs = BBQOption::OOEggs::Reroll;
    std::vector<BBQuests> expected_to_do;
    std::vector<size_t> expected_reroll_slots;
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


// A quest panel with a cursor. A reroll replaces the quest under the cursor
// with the next of `replacements`, unless out of BP.
class SimulatedQuestPanel : public BBQRerollActions{
public:
    std::vector<BBQuests> quests;
    std::vector<BBQuests> replacements;
    size_t cursor = 0;
    bool panel_opens = true;
    bool menu_seen = true;
    bool out_of_bp = false;
    int missed_reads = 0;   // Reads that fail OCR on every slot.

    bool panel_open = false;
    bool menu_open = false;
    std::vector<size_t> rerolled_slots;

    virtual bool open_panel() override{
        panel_open = panel_opens;
        return panel_open;
    }
    virtual std::optional<size_t> open_reroll_menu() override{
        menu_open = true;
        if (!menu_seen){
            return std::nullopt;
        }
        return cursor;
    }
    virtual void close_reroll_menu() override{
        menu_open = false;
    }
    virtual void move_cursor(int slots) override{
        cursor = (size_t)((int)cursor + slots);
    }
    virtual void confirm_reroll() override{
        menu_open = false;
        if (out_of_bp){
            return;
        }
        rerolled_slots.push_back(cursor);
        quests[cursor] = replacements[rerolled_slots.size() - 1];
    }
    virtual void close_panel() override{
        panel_open = false;
        menu_open = false;
    }
    virtual std::vector<BBQuests> read_quests() override{
        if (missed_reads > 0){
            missed_reads--;
            return std::vector<BBQuests>(quests.size(), BBQuests::UnableToDetect);
        }
        return quests;
    }
};

struct BBQRerollScenario{
    std::string name;
    std::vector<BBQuests> quests;
    std::vector<BBQuests> replacements;
    size_t slot;
    size_t cursor = 0;
    bool panel_opens = true;
    bool menu_seen = true;
    bool out_of_bp = false;
    int missed_reads = 0;
    BBQRerollResult expected;
    std::vector<size_t> expected_rerolled_slots;
    std::vector<BBQuests> expected_quests;
};

std::string reroll_result_to_string(BBQRerollResult result){
    switch (result){
    case BBQRerollResult::Rerolled:         return "Rerolled";
    case BBQRerollResult::PanelNotOpened:   return "PanelNotOpened";
    case BBQRerollResult::MenuNotFound:     return "MenuNotFound";
    case BBQRerollResult::NotReplaced:      return "NotReplaced";
    }
    return "?";
}

class Test_BBQReroll : public UnitTest{
public:
    Test_BBQReroll(BBQRerollScenario scenario)
        : UnitTest("PokemonSV::BBQReroll - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        SimulatedQuestPanel panel;
        panel.quests = m_scenario.quests;
        panel.replacements = m_scenario.replacements;
        panel.cursor = m_scenario.cursor;
        panel.panel_opens = m_scenario.panel_opens;
        panel.menu_seen = m_scenario.menu_seen;
        panel.out_of_bp = m_scenario.out_of_bp;
        panel.missed_reads = m_scenario.missed_reads;

        NullLogger logger;
        std::vector<BBQuests> quests = m_scenario.quests;
        const BBQRerollResult result = reroll_quest(logger, panel, quests, m_scenario.slot);
        if (result != m_scenario.expected){
            return "Expected " + reroll_result_to_string(m_scenario.expected)
                + ", got " + reroll_result_to_string(result);
        }
        if (panel.rerolled_slots != m_scenario.expected_rerolled_slots){
            return "Expected rerolled slots: " + slots_to_string(m_scenario.expected_rerolled_slots)
                + "got: " + slots_to_string(panel.rerolled_slots);
        }
        if (quests != m_scenario.expected_quests){
            return "Expected quests: " + quests_to_string(m_scenario.expected_quests)
                + "got: " + quests_to_string(quests);
        }
        if (panel.panel_open || panel.menu_open){
            return "Panel left open.";
        }
        return true;
    }

private:
    const BBQRerollScenario m_scenario;
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
            // One quest costs less BP to reroll than all of them.
            .name = "nothing doable rerolls the first quest not possible",
            .quests = {auto_10, pickup_10, catch_flying},
            .expected_reroll_slots = {1},
        },
        {
            .name = "nothing doable and nothing not possible rerolls all",
            .quests = {auto_10, sneak_up},
            .extra_exclusions = {{sneak_up, BBQAction::skip}},
            .expected_reroll_all = true,
        },
        {
            // An undetected quest is not known to be impossible.
            .name = "an undetected quest is not rerolled alone",
            .quests = {UnableToDetect, auto_10},
            .expected_reroll_all = true,
        },
        {
            .name = "a skipped quest not possible is not rerolled",
            .quests = {pickup_10, auto_10, catch_flying},
            .extra_exclusions = {{pickup_10, BBQAction::skip}},
            .expected_reroll_slots = {2},
        },
        {
            .name = "a reroll row rerolls a quest not possible",
            .quests = {make_tm, sneak_up, pickup_10},
            .extra_exclusions = {{pickup_10, BBQAction::reroll}},
            .expected_to_do = {make_tm, sneak_up},
            .expected_reroll_slots = {2},
        },
        {
            .name = "a run row does not run a quest not possible",
            .quests = {make_tm, pickup_10},
            .extra_exclusions = {{pickup_10, BBQAction::run}},
            .expected_to_do = {make_tm},
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

void add_tests_BBQReroll(UnitTestDatabase& database){
    using enum BBQuests;
    const BBQRerollScenario scenarios[] = {
        {
            .name = "cursor on the top quest",
            .quests = {make_tm, sneak_up, travel_500},
            .replacements = {catch_any},
            .slot = 2,
            .expected = BBQRerollResult::Rerolled,
            .expected_rerolled_slots = {2},
            .expected_quests = {make_tm, sneak_up, catch_any},
        },
        {
            // Seen on a Switch: the panel can open with the cursor where it
            // was left, so moving down from the top rerolls the wrong quest.
            .name = "cursor left below the quest moves up",
            .quests = {make_tm, travel_500, sneak_up, hatch_egg},
            .replacements = {catch_any},
            .slot = 1,
            .cursor = 3,
            .expected = BBQRerollResult::Rerolled,
            .expected_rerolled_slots = {1},
            .expected_quests = {make_tm, catch_any, sneak_up, hatch_egg},
        },
        {
            .name = "fourth quest",
            .quests = {make_tm, travel_500, sneak_up, hatch_egg},
            .replacements = {catch_any},
            .slot = 3,
            .expected = BBQRerollResult::Rerolled,
            .expected_rerolled_slots = {3},
            .expected_quests = {make_tm, travel_500, sneak_up, catch_any},
        },
        {
            .name = "panel not opened spends nothing",
            .quests = {make_tm, sneak_up, travel_500},
            .slot = 2,
            .panel_opens = false,
            .expected = BBQRerollResult::PanelNotOpened,
            .expected_quests = {make_tm, sneak_up, travel_500},
        },
        {
            .name = "menu not seen spends nothing",
            .quests = {make_tm, sneak_up, travel_500},
            .slot = 2,
            .menu_seen = false,
            .expected = BBQRerollResult::MenuNotFound,
            .expected_quests = {make_tm, sneak_up, travel_500},
        },
        {
            .name = "out of BP is not replaced",
            .quests = {make_tm, sneak_up, travel_500},
            .slot = 2,
            .out_of_bp = true,
            .expected = BBQRerollResult::NotReplaced,
            .expected_quests = {make_tm, sneak_up, travel_500},
        },
        {
            .name = "a missed read is read again",
            .quests = {make_tm, sneak_up, travel_500},
            .replacements = {catch_any},
            .slot = 2,
            .missed_reads = 2,
            .expected = BBQRerollResult::Rerolled,
            .expected_rerolled_slots = {2},
            .expected_quests = {make_tm, sneak_up, catch_any},
        },
        {
            .name = "three missed reads give up",
            .quests = {make_tm, sneak_up, travel_500},
            .replacements = {catch_any},
            .slot = 2,
            .missed_reads = 3,
            .expected = BBQRerollResult::NotReplaced,
            .expected_rerolled_slots = {2},
            .expected_quests = {make_tm, sneak_up, travel_500},
        },
        {
            // Cannot be told apart from out of BP.
            .name = "replaced by the same quest is not replaced",
            .quests = {make_tm, sneak_up, travel_500},
            .replacements = {travel_500},
            .slot = 2,
            .expected = BBQRerollResult::NotReplaced,
            .expected_rerolled_slots = {2},
            .expected_quests = {make_tm, sneak_up, travel_500},
        },
    };
    for (const BBQRerollScenario& scenario : scenarios){
        database.add<Test_BBQReroll>(scenario);
    }
}



void add_tests(UnitTestDatabase& database){
    add_tests_BBQuestPlan(database);
    add_tests_BBQReroll(database);
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
