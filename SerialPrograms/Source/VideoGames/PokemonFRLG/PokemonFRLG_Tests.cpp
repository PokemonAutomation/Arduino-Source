/*  Pokemon FRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <fstream>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/StaticGlobals.h"
#include "Tests/TestUtils.h"
#include "Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "Inference/Dialogs/PokemonFRLG_PrizeSelectDetector.h"
#include "Inference/Map/PokemonFRLG_PokemonLeagueDetectors.h"
#include "Inference/Menus/PokemonFRLG_BagDetector_Tests.h"
#include "Inference/Menus/PokemonFRLG_PartyEmptySlotDetector.h"
#include "Inference/Menus/PokemonFRLG_PartyHeldItemDetector.h"
#include "Inference/Menus/PokemonFRLG_PartyMenuDetector.h"
#include "Inference/PokemonFRLG_BattleLevelUpReader.h"
#include "Inference/PokemonFRLG_BattleSelectionArrowDetector.h"
#include "Inference/PokemonFRLG_PartyLevelUpReader.h"
#include "Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "Inference/PokemonFRLG_StatsReader.h"
#include "Inference/PokemonFRLG_TrainerIdReader.h"
#include "Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG_Navigation.h"

#include "PokemonFRLG_Tests.h"
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


namespace{

class NullLogger : public Logger{
public:
    virtual void log(const std::string&, Color) override{}
};

std::string button_name(Button button){
    return button == BUTTON_A ? "A" : button == BUTTON_B ? "B" : "?";
}

// Steps that return scripted events, and record what exit_wild_battle() asked for.
class ScriptedWildBattleExit : public WildBattleExitActions{
public:
    // Returned by the press steps in turn; None once used up. Mashing B only watches for the
    // battle to end, so it takes the next event only if that is BattleExited.
    std::vector<WildBattleExitEvent> events;
    // Returned by evolution_started() in turn; false once used up.
    std::vector<bool> evolutions;
    // How many overworld checks fail before one finds it.
    int overworld_misses = 0;

    std::vector<std::string> steps;
    // Seconds spent in steps that ran to their timeout, and waiting for an evolution: time in
    // which the player sees nothing happen.
    int idle_seconds = 0;

    virtual WildBattleExitEvent mash_b_until_battle_exited() override{
        steps.push_back("mash-B");
        if (m_next_event < events.size() && events[m_next_event] == WildBattleExitEvent::BattleExited){
            return events[m_next_event++];
        }
        idle_seconds += 20;
        return WildBattleExitEvent::None;
    }
    virtual WildBattleExitEvent press_until_dialog(Button button) override{
        steps.push_back("press-" + button_name(button));
        WildBattleExitEvent event = next_event();
        if (event == WildBattleExitEvent::None){
            idle_seconds += 30;
        }
        return event;
    }
    virtual bool evolution_started() override{
        steps.push_back("evolution?");
        return m_next_evolution < evolutions.size() && evolutions[m_next_evolution++];
    }
    virtual void wait_for_evolution() override{
        steps.push_back("wait");
        idle_seconds += 20;
    }
    virtual void advance_dialog() override{
        steps.push_back("advance");
    }
    virtual void answer_move_prompt(Button button) override{
        steps.push_back("answer-" + button_name(button));
    }
    virtual bool return_to_overworld() override{
        steps.push_back("overworld?");
        return overworld_misses-- <= 0;
    }

private:
    WildBattleExitEvent next_event(){
        return m_next_event < events.size() ? events[m_next_event++] : WildBattleExitEvent::None;
    }

    size_t m_next_event = 0;
    size_t m_next_evolution = 0;
};

struct WildBattleExitScenario{
    std::string name;
    std::vector<WildBattleExitEvent> events = {};
    std::vector<bool> evolutions = {};
    int overworld_misses = 0;
    bool stop_on_move_learn = false;
    bool prevent_evolution = false;
    WildBattleExitResult expected_result = WildBattleExitResult::Exited;
    std::string expected_steps;
    int expected_idle_seconds = 0;
};

std::string result_name(WildBattleExitResult result){
    switch (result){
    case WildBattleExitResult::Exited:                 return "Exited";
    case WildBattleExitResult::ExitedAfterMoveLearn:   return "ExitedAfterMoveLearn";
    case WildBattleExitResult::StoppedOnMoveLearn:     return "StoppedOnMoveLearn";
    case WildBattleExitResult::Failed:                 return "Failed";
    }
    return "?";
}

class Test_ExitWildBattle : public UnitTest{
public:
    Test_ExitWildBattle(WildBattleExitScenario scenario)
        : UnitTest("PokemonFRLG::ExitWildBattle - " + scenario.name)
        , m_scenario(std::move(scenario))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        ScriptedWildBattleExit actions;
        actions.events = m_scenario.events;
        actions.evolutions = m_scenario.evolutions;
        actions.overworld_misses = m_scenario.overworld_misses;

        NullLogger logger;
        const WildBattleExitResult result = exit_wild_battle(
            logger, actions, m_scenario.stop_on_move_learn, m_scenario.prevent_evolution
        );
        if (result != m_scenario.expected_result){
            return "Expected " + result_name(m_scenario.expected_result) + ", got " + result_name(result);
        }
        std::string steps;
        for (const std::string& step : actions.steps){
            if (!steps.empty()){
                steps += ' ';
            }
            steps += step;
        }
        if (steps != m_scenario.expected_steps){
            return "Expected steps: " + m_scenario.expected_steps + "\nActual steps:   " + steps;
        }
        if (actions.idle_seconds != m_scenario.expected_idle_seconds){
            return "Expected " + std::to_string(m_scenario.expected_idle_seconds) + " idle seconds, got "
                + std::to_string(actions.idle_seconds);
        }
        return true;
    }

private:
    const WildBattleExitScenario m_scenario;
};

void add_tests_ExitWildBattle(UnitTestDatabase& database){
    using enum WildBattleExitEvent;
    const WildBattleExitScenario scenarios[] = {
        {
            .name = "battle exits",
            .events = {BattleExited},
            .expected_steps = "mash-B evolution?",
        },
        {
            // Mashing B did not end the battle, so it presses B until a dialog shows.
            .name = "dialog is advanced",
            .events = {AdvanceDialog, BattleExited},
            .expected_steps = "mash-B press-B advance press-B evolution?",
            .expected_idle_seconds = 20,
        },
        {
            // Decline "forget a move?" with B, then confirm "stop learning?" with A.
            .name = "new move is declined",
            .events = {MoveLearnPrompt, MoveLearnPrompt, BattleExited},
            .expected_result = WildBattleExitResult::ExitedAfterMoveLearn,
            .expected_steps = "mash-B press-B answer-B press-A answer-A press-A evolution?",
            .expected_idle_seconds = 20,
        },
        {
            .name = "stops at a new move when asked",
            .events = {MoveLearnPrompt},
            .stop_on_move_learn = true,
            .expected_result = WildBattleExitResult::StoppedOnMoveLearn,
            .expected_steps = "mash-B press-B",
            .expected_idle_seconds = 20,
        },
        {
            // After an evolution nothing ends the battle again, so the overworld is checked
            // right after mashing B, without pressing B until the deadline first.
            .name = "evolution",
            .events = {BattleExited},
            .evolutions = {true},
            .expected_steps = "mash-B evolution? wait mash-B overworld?",
            .expected_idle_seconds = 40,
        },
        {
            // Mashing B right away cancels the evolution.
            .name = "prevented evolution",
            .events = {BattleExited},
            .evolutions = {true},
            .prevent_evolution = true,
            .expected_steps = "mash-B evolution? mash-B overworld?",
            .expected_idle_seconds = 20,
        },
        {
            // A move to learn after the evolution keeps the overworld from being found, so it
            // goes back to pressing B, which finds the move-learn prompt.
            .name = "evolution then new move is declined",
            .events = {BattleExited, MoveLearnPrompt, MoveLearnPrompt},
            .evolutions = {true},
            .overworld_misses = 1,
            .expected_result = WildBattleExitResult::ExitedAfterMoveLearn,
            .expected_steps =
                "mash-B evolution? wait mash-B overworld? "
                "press-B answer-B press-A answer-A press-A overworld?",
            .expected_idle_seconds = 70,
        },
        {
            // If the overworld is not found after the evolution, it goes back to pressing B.
            .name = "evolution and overworld not found",
            .events = {BattleExited},
            .evolutions = {true},
            .overworld_misses = 100,
            .expected_result = WildBattleExitResult::Failed,
            .expected_steps =
                "mash-B evolution? wait mash-B overworld? "
                "press-B overworld? press-B overworld? press-B overworld? "
                "press-B overworld? press-B overworld? press-B overworld?",
            .expected_idle_seconds = 220,
        },
        {
            .name = "gives up when the overworld is not found",
            .overworld_misses = 100,
            .expected_result = WildBattleExitResult::Failed,
            .expected_steps =
                "mash-B press-B overworld? press-B overworld? press-B overworld? "
                "press-B overworld? press-B overworld? press-B overworld?",
            .expected_idle_seconds = 200,
        },
    };
    for (const WildBattleExitScenario& scenario : scenarios){
        database.add<Test_ExitWildBattle>(scenario);
    }
}

}


void add_tests(UnitTestDatabase& database){
    add_tests_AdvanceWhiteDialogDetector(database);
    add_tests_SelectionDialogDetector(database);
    add_tests_ShinySymbolDetector(database);
    add_tests_PrizeSelectDetector(database);
    add_tests_AdvanceBattleDialogDetector(database);
    add_tests_BattleMenuDetector(database);
    add_tests_StatsReader(database);
    add_tests_WildEncounterReader(database);
    add_tests_TrainerIdReader(database);
    add_tests_PartyLevelUpReader(database);
    add_tests_BattleLevelUpReader(database);
    add_tests_PartyEmptySlotDetector(database);
    add_tests_PartyHeldItemDetector(database);
    add_tests_PartyMenuDetector(database);
    add_tests_PartySlotDetector(database);
    add_tests_EliteFourRoomDetector(database);
    add_tests_HallOfFameSavingDetector(database);
    add_tests_BattleMoveSelectionArrow(database);
    add_tests_BagDetector(database);
    add_tests_ExitWildBattle(database);
}


UnitTestResult check_against_golden_file(
    const std::string& image_path,
    const std::vector<std::string>& labels,
    const std::vector<std::string>& values
){
    if (labels.size() != values.size()){
        return "Error: check_against_golden_file() called with " +
               std::to_string(labels.size()) + " labels for " +
               std::to_string(values.size()) + " values.";
    }
    for (size_t c = 0; c < values.size(); c++){
        if (values[c].empty() || values[c].find_first_of(" \t\r\n") != std::string::npos){
            return "Error: value for \"" + labels[c] + "\" is empty or contains whitespace. "
                   "The golden file cannot represent it.";
        }
    }

    Filesystem::Path file_path(image_path);
    Filesystem::Path golden_path =
            file_path.parent_path() / ("_" + file_path.stem().string() + ".txt");

    if (STATIC_GLOBALS.GENERATE_TEST_GOLDEN_FILES){
        std::ofstream output_file(golden_path.stdpath());
        if (!output_file.is_open()){
            return "Error: cannot open " + golden_path.string() + " for writing.";
        }
        for (const std::string& value : values){
            output_file << value << std::endl;
        }
        return true;
    }

    std::vector<std::string> targets;
    if (!load_slug_list(golden_path.string(), targets)){
        return "Error: cannot load golden file " + golden_path.string() + ".";
    }
    if (targets.size() != values.size()){
        return "Error: golden file " + golden_path.string() + " has " +
               std::to_string(targets.size()) + " values, expected " +
               std::to_string(values.size()) + ".";
    }
    for (size_t c = 0; c < values.size(); c++){
        TEST_RESULT_COMPONENT_EQUAL_STR(values[c], targets[c], labels[c]);
    }
    return true;
}


}
}
}