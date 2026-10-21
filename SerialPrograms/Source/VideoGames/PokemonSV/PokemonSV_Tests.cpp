/*  Pokemon SV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <set>
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
#include "Programs/Farming/PokemonSV_BBQSoloFarmer.h"
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


namespace{

// A singleplayer quest panel that behaves like the game's: 3 blue quests,
// and a completed quest is replaced by a new one. process_quest_list()
// follows the two rules of the real one that the round loop depends on:
// quests in not_possible_quests are skipped, and if nothing is doable all
// quests are rerolled and nothing is returned. Red quests and the
// exclusions table are not modeled. Records the game actions taken.
class SimulatedBBQGameActions : public BBQGameActions{
public:
    std::vector<BBQuests> panel;
    // Each attempt returns the next outcome; once exhausted, attempts succeed.
    std::vector<bool> outcomes;
    // Completing the first quest also completes the second, e.g. walking
    // 500 meters while navigating to a photo target.
    std::vector<std::pair<BBQuests, BBQuests>> also_completes;
    // Attempts at these always fail, e.g. a target that never spawns.
    std::set<BBQuests> never_completes;
    // New quests are drawn from this list in turn, skipping any that are
    // already on the panel.
    std::vector<BBQuests> new_quests = {
        BBQuests::photo_canyon, BBQuests::photo_coastal, BBQuests::photo_polar,
        BBQuests::photo_savanna, BBQuests::catch_any, BBQuests::catch_normal,
        BBQuests::catch_fire, BBQuests::catch_water, BBQuests::catch_grass,
    };

    std::vector<std::string> events;

    virtual std::vector<BBQuests> read_quests() override{
        events.push_back("read");
        if (++m_reads > 1000){
            throw std::runtime_error("Too many quest panel reads.");
        }
        return panel;
    }
    virtual std::vector<BBQuests> process_quest_list(
        const std::vector<BBQuests>& quests, uint8_t
    ) override{
        events.push_back("process");
        std::vector<BBQuests> doable;
        for (BBQuests quest : quests){
            if (!not_possible_quests.contains(quest)){
                doable.push_back(quest);
            }
        }
        if (doable.empty()){
            for (BBQuests& slot : panel){
                slot = next_new_quest();
            }
        }
        return doable;
    }
    virtual bool do_quest(BBQuests quest, uint8_t&) override{
        events.push_back("do");
        const bool success = !never_completes.contains(quest)
            && (m_next_outcome < outcomes.size() ? outcomes[m_next_outcome++] : true);
        if (success){
            complete(quest);
            for (const auto& [done, extra] : also_completes){
                if (done == quest){
                    complete(extra);
                }
            }
        }
        return success;
    }
    virtual void reroll_quest(BBQuests quest) override{
        events.push_back("reroll");
        auto iter = std::find(panel.begin(), panel.end(), quest);
        if (iter != panel.end()){
            *iter = next_new_quest();
        }
    }
    virtual void quest_completed() override{
        events.push_back("completed");
    }
    virtual void end_round(bool save) override{
        events.push_back(save ? "end(save)" : "end");
    }

private:
    void complete(BBQuests quest){
        auto iter = std::find(panel.begin(), panel.end(), quest);
        if (iter != panel.end()){
            *iter = next_new_quest();
        }
    }

    BBQuests next_new_quest(){
        while (true){
            BBQuests quest = new_quests[m_next_new_quest++ % new_quests.size()];
            if (std::find(panel.begin(), panel.end(), quest) == panel.end()){
                return quest;
            }
        }
    }

    size_t m_reads = 0;
    size_t m_next_outcome = 0;
    size_t m_next_new_quest = 0;
};

std::string join_events(const std::vector<std::string>& events){
    std::string ret;
    for (const std::string& event : events){
        if (!ret.empty()) ret += ' ';
        ret += event;
    }
    return ret;
}

// One simulated farmer run: option values, the panel it starts with and how
// quests turn out, and the game actions it must take.
struct BBQRoundsScenario{
    std::string name;
    uint64_t num_quests;
    uint64_t save_num_quests = 0;
    bool stop_when_out_of_eggs = false;
    std::vector<BBQuests> panel = {BBQuests::make_tm, BBQuests::auto_10, BBQuests::sneak_up};
    std::vector<bool> outcomes;
    std::vector<std::pair<BBQuests, BBQuests>> also_completes;
    std::set<BBQuests> never_completes;
    std::vector<BBQuests> new_quests;
    BBQOption::OutOfRetries out_of_retries = BBQOption::OutOfRetries::Skip;
    uint64_t expected_completed = 0;
    std::string expected;
};

class Test_BBQFarmerRounds : public UnitTest{
public:
    Test_BBQFarmerRounds(BBQRoundsScenario scenario)
        : UnitTest("PokemonSV::BBQFarmerRounds - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        BBQOption options;
        options.NUM_QUESTS.set(m_scenario.num_quests);
        options.SAVE_NUM_QUESTS.set(m_scenario.save_num_quests);
        if (m_scenario.stop_when_out_of_eggs){
            options.NUM_EGGS.set(0);
            options.OUT_OF_EGGS.set(BBQOption::OOEggs::Stop);
        }
        options.OUT_OF_RETRIES.set(m_scenario.out_of_retries);

        SimulatedBBQGameActions game;
        game.panel = m_scenario.panel;
        game.outcomes = m_scenario.outcomes;
        game.also_completes = m_scenario.also_completes;
        game.never_completes = m_scenario.never_completes;
        if (!m_scenario.new_quests.empty()){
            game.new_quests = m_scenario.new_quests;
        }

        NullLogger logger;
        run_bbq_rounds(logger, options, game);
        const uint64_t completed = (uint64_t)std::count(game.events.begin(), game.events.end(), "completed");
        if (completed != m_scenario.expected_completed){
            return "Expected " + std::to_string(m_scenario.expected_completed)
                + " completed quests, counted " + std::to_string(completed);
        }
        const std::string actual = join_events(game.events);
        if (actual != m_scenario.expected){
            return "Expected: " + m_scenario.expected + "\nActual:   " + actual;
        }
        return true;
    }

private:
    const BBQRoundsScenario m_scenario;
};

void add_tests_BBQFarmerRounds(UnitTestDatabase& database){
    const std::string round_of_3 = "read process read do completed read do completed read do completed";
    const BBQRoundsScenario scenarios[] = {
        {
            // The goal (4) is reached in the middle of round 2. The loop only
            // checks the goal between rounds, so round 2 finishes and 6 quests
            // complete.
            .name = "finishes the round that reaches the quest goal",
            .num_quests = 4,
            .expected_completed = 6,
            .expected = round_of_3 + " end " + round_of_3 + " end",
        },
        {
            // Doing photo_swim also completes travel_500, so travel_500 is
            // gone when the loop reads the panel before attempting it.
            .name = "quest already gone counts as completed",
            .num_quests = 3,
            .panel = {BBQuests::photo_swim, BBQuests::travel_500, BBQuests::make_tm},
            .also_completes = {{BBQuests::photo_swim, BBQuests::travel_500}},
            .expected_completed = 3,
            .expected = "read process read do completed read completed read do completed end",
        },
        {
            // The first attempt fails: 3 attempts, 2 completions.
            .name = "failed quest does not count",
            .num_quests = 2,
            .outcomes = {false},
            .expected_completed = 2,
            .expected = "read process read do read do completed read do completed end",
        },
        {
            // Nothing on the panel is doable, so process_quest_list() rerolls
            // all of it and returns nothing; the loop reads the panel again.
            .name = "panel with nothing doable is rerolled",
            .num_quests = 3,
            .panel = {BBQuests::pickup_10, BBQuests::catch_flying, BBQuests::UnableToDetect},
            .expected_completed = 3,
            .expected = "read process " + round_of_3 + " end",
        },
        {
            .name = "saves at exact multiples",
            .num_quests = 6,
            .save_num_quests = 3,
            .expected_completed = 6,
            .expected = round_of_3 + " end(save) " + round_of_3 + " end(save)",
        },
        {
            .name = "stops when out of eggs",
            .num_quests = 5,
            .stop_when_out_of_eggs = true,
            .expected_completed = 0,
            .expected = "",
        },
        {
            // sneak-up fails in round 1 and is still on the panel in round 2,
            // where only the two new quests are attempted.
            .name = "failed quest is not attempted again",
            .num_quests = 4,
            .never_completes = {BBQuests::sneak_up},
            .expected_completed = 4,
            .expected =
                "read process read do completed read do completed read do end "
                "read process read do completed read do completed end",
        },
        {
            .name = "stops when only failed quests are doable",
            .num_quests = 3,
            .panel = {BBQuests::sneak_up, BBQuests::pickup_10, BBQuests::catch_flying},
            .never_completes = {BBQuests::sneak_up},
            .expected_completed = 0,
            .expected = "read process read do end read process",
        },
        {
            // sneak-up fails, then leaves the panel along with make-tm. Round 2
            // reads a full panel without it, and its replacement for
            // photo-canyon brings it back. Round 3 attempts it again.
            .name = "failed quest that leaves a full panel is attempted again",
            .num_quests = 5,
            .panel = {BBQuests::sneak_up, BBQuests::make_tm, BBQuests::pickup_10, BBQuests::catch_flying},
            .outcomes = {false},
            .also_completes = {{BBQuests::make_tm, BBQuests::sneak_up}},
            .new_quests = {BBQuests::auto_10, BBQuests::photo_canyon, BBQuests::sneak_up},
            .expected_completed = 5,
            .expected =
                "read process read do read do completed end "
                "read process read do completed read do completed end "
                "read process read do completed read do completed end",
        },
        {
            // As above with 3 slots. A 4th quest could have been missed by
            // OCR, so the read does not prove sneak-up left: it stays failed.
            .name = "failed quest missing from a 3-slot panel stays failed",
            .num_quests = 4,
            .panel = {BBQuests::sneak_up, BBQuests::make_tm, BBQuests::pickup_10},
            .outcomes = {false},
            .also_completes = {{BBQuests::make_tm, BBQuests::sneak_up}},
            .new_quests = {BBQuests::auto_10, BBQuests::photo_canyon, BBQuests::sneak_up},
            .expected_completed = 4,
            .expected =
                "read process read do read do completed end "
                "read process read do completed read do completed end "
                "read process read do completed end",
        },
        {
            // sneak-up is rerolled to photo-polar, which round 2 attempts.
            .name = "out of retries with reroll replaces the failed quest",
            .num_quests = 3,
            .never_completes = {BBQuests::sneak_up},
            .out_of_retries = BBQOption::OutOfRetries::Reroll,
            .expected_completed = 5,
            .expected = "read process read do completed read do completed read do reroll end " + round_of_3 + " end",
        },
        {
            .name = "out of retries with stop stops at once",
            .num_quests = 3,
            .panel = {BBQuests::sneak_up, BBQuests::make_tm, BBQuests::auto_10},
            .never_completes = {BBQuests::sneak_up},
            .out_of_retries = BBQOption::OutOfRetries::Stop,
            .expected_completed = 0,
            .expected = "read process read do",
        },
    };
    for (const BBQRoundsScenario& scenario : scenarios){
        database.add<Test_BBQFarmerRounds>(scenario);
    }
}

}

void add_tests(UnitTestDatabase& database){
    add_tests_BBQuestPlan(database);
    add_tests_BBQReroll(database);
    add_tests_BBQFarmerRounds(database);
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
