/*  Pokemon SV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
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

// A singleplayer quest panel that behaves like the game's: 3 blue quests,
// and a completed quest is replaced by a new one. process_quest_list()
// follows the two rules of the real one that the round loop depends on:
// quests in not_possible_quests are skipped, and if nothing is doable all
// quests are rerolled and nothing is returned. Red quests and the
// exclusions table are not modeled. Records the game actions taken.
// With out_of_bp, rerolls do nothing, as in the game when BP runs out.
class SimulatedBBQGameActions : public BBQGameActions{
public:
    std::vector<BBQuests> panel;
    // Each attempt returns the next outcome; once exhausted, attempts succeed.
    std::vector<bool> outcomes;
    // Completing the first quest also completes the second, e.g. walking
    // 500 meters while navigating to a photo target.
    std::vector<std::pair<BBQuests, BBQuests>> also_completes;
    bool out_of_bp = false;

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
        if (doable.empty() && !out_of_bp){
            for (BBQuests& slot : panel){
                slot = next_new_quest();
            }
        }
        return doable;
    }
    virtual bool do_quest(BBQuests quest, uint8_t&) override{
        events.push_back("do");
        const bool success = m_next_outcome < outcomes.size() ? outcomes[m_next_outcome++] : true;
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

    // New quests come from a fixed rotation of doable blue quests, skipping
    // any that are already on the panel.
    BBQuests next_new_quest(){
        static const BBQuests ROTATION[] = {
            BBQuests::photo_canyon, BBQuests::photo_coastal, BBQuests::photo_polar,
            BBQuests::photo_savanna, BBQuests::catch_any, BBQuests::catch_normal,
            BBQuests::catch_fire, BBQuests::catch_water, BBQuests::catch_grass,
        };
        while (true){
            BBQuests quest = ROTATION[m_next_new_quest++ % std::size(ROTATION)];
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
    std::vector<std::pair<BBQuests, BBQAction>> exclusions;
    bool out_of_bp = false;
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
        for (const auto& [quest, action] : m_scenario.exclusions){
            auto row = std::make_unique<BBQuestTableRow>(options.QUEST_EXCLUSIONS);
            row->quest.set(quest);
            row->action.set(action);
            options.QUEST_EXCLUSIONS.append_row(std::move(row));
        }

        SimulatedBBQGameActions game;
        game.panel = m_scenario.panel;
        game.outcomes = m_scenario.outcomes;
        game.also_completes = m_scenario.also_completes;
        game.out_of_bp = m_scenario.out_of_bp;

        NullLogger logger;
        try{
            run_bbq_rounds(logger, options, game);
        }catch (UserSetupError&){
            game.events.push_back("error");
        }
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

// Every blue quest the program can do, each with the given action.
std::vector<std::pair<BBQuests, BBQAction>> every_doable_blue_quest(BBQAction action){
    std::vector<std::pair<BBQuests, BBQAction>> ret;
    for (BBQuests quest : blue_quests){
        if (!not_possible_quests.contains(quest)){
            ret.emplace_back(quest, action);
        }
    }
    return ret;
}

void add_tests_BBQFarmerRounds(UnitTestDatabase& database){
    const std::string round_of_3 = "read process read do completed read do completed read do completed";
    std::string read_30_times;
    for (int c = 0; c < 30; c++){
        read_30_times += "read process ";
    }
    std::vector<std::pair<BBQuests, BBQAction>> all_skipped_but_make_tm = every_doable_blue_quest(BBQAction::skip);
    all_skipped_but_make_tm.emplace_back(BBQuests::make_tm, BBQAction::run);
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
            // Rerolling could never produce a doable panel, so the run
            // fails before reading the panel.
            .name = "fails at start when every blue quest is excluded",
            .num_quests = 3,
            .exclusions = every_doable_blue_quest(BBQAction::reroll),
            .expected_completed = 0,
            .expected = "error",
        },
        {
            // A run row keeps the quest doable even when another row skips it.
            .name = "a quest with a run row is not excluded",
            .num_quests = 3,
            .exclusions = all_skipped_but_make_tm,
            .expected_completed = 3,
            .expected = round_of_3 + " end",
        },
        {
            // Rerolls change nothing, so the panel never becomes doable.
            .name = "gives up when rerolling does not help",
            .num_quests = 3,
            .panel = {BBQuests::pickup_10, BBQuests::catch_flying, BBQuests::UnableToDetect},
            .out_of_bp = true,
            .expected_completed = 0,
            .expected = read_30_times + "error",
        },
    };
    for (const BBQRoundsScenario& scenario : scenarios){
        database.add<Test_BBQFarmerRounds>(scenario);
    }
}

}

void add_tests(UnitTestDatabase& database){
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
