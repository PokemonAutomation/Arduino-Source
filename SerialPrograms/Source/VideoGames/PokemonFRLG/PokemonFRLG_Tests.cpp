/*  Pokemon FRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <string_view>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/StaticGlobals.h"
#include "Inference/Box/PokemonFRLG_BoxCellOccupiedDetector.h"
#include "Inference/Box/PokemonFRLG_BoxCellReader.h"
#include "Inference/Box/PokemonFRLG_BoxGenderReader.h"
#include "Inference/Box/PokemonFRLG_BoxSelectionModeDetector.h"
#include "Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "Inference/Dialogs/PokemonFRLG_PrizeSelectDetector.h"
#include "Inference/Map/PokemonFRLG_PokemonLeagueDetectors.h"
#include "Inference/Menus/PokemonFRLG_BagDetector.h"
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
#include "Programs/BoxOrganization/PokemonFRLG_BoxSorter_Tests.h"
#include "Programs/RngManipulation/PokemonFRLG_EggRng.h"
#include "Programs/RngManipulation/PokemonFRLG_GiftRng.h"
#include "Programs/RngManipulation/PokemonFRLG_RngHelper.h"
#include "Programs/RngManipulation/PokemonFRLG_RngProgram.h"
#include "Programs/RngManipulation/PokemonFRLG_RoamingLegendaryRng.h"
#include "Programs/RngManipulation/PokemonFRLG_StarterRng.h"
#include "Programs/RngManipulation/PokemonFRLG_StaticRng.h"
#include "Programs/RngManipulation/PokemonFRLG_WildRng.h"
#include "PokemonFRLG_Navigation.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"

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


std::string slot_name(PartySlot slot){
    return std::to_string(static_cast<unsigned>(slot) + 1);
}

// Reads the last occupied slot from a party menu screenshot, as the RNG programs' party check does.
class Test_LastOccupiedPartySlot : public ImageUnitTest{
public:
    Test_LastOccupiedPartySlot(std::string_view image, PartySlot expected)
        : ImageUnitTest("PokemonFRLG::LastOccupiedPartySlot - " + std::string(image), {std::string(image)})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        const PartySlot slot = detect_last_occupied_party_slot(images[0]);
        if (slot != m_expected){
            return std::format("Expected last occupied slot {}, got {}", slot_name(m_expected), slot_name(slot));
        }
        return true;
    }

private:
    const PartySlot m_expected;
};

void add_tests_LastOccupiedPartySlot(UnitTestDatabase& database){
    for (unsigned occupied = 1; occupied <= 6; occupied++){
        database.add<Test_LastOccupiedPartySlot>(
            "PokemonFRLG/Party/Party_" + std::to_string(occupied) + ".png",
            static_cast<PartySlot>(occupied - 1)
        );
    }
    database.add<Test_LastOccupiedPartySlot>("PokemonFRLG/Party/Party_6_With_Held_Items.png", PartySlot::SIX);
    database.add<Test_LastOccupiedPartySlot>("PokemonFRLG/Party/JPN_HeldItem.png", PartySlot::SIX);
    database.add<Test_LastOccupiedPartySlot>("PokemonFRLG/Party/JPN_Party.png", PartySlot::FOUR);
    database.add<Test_LastOccupiedPartySlot>("PokemonFRLG/Party/SPA_HeldItem.png", PartySlot::SIX);
}

// Steps that return scripted reads, and record what check_party_free_slots() asked for.
class ScriptedPartyCheck : public PartyCheckActions{
public:
    bool menu_opens = true;
    PartySlot last_occupied = PartySlot::ONE;
    std::string steps;

    virtual bool open_party_menu() override{
        steps += steps.empty() ? "open" : " open";
        return menu_opens;
    }
    virtual PartySlot read_last_occupied_slot() override{
        steps += steps.empty() ? "read" : " read";
        return last_occupied;
    }
};

struct PartyCheckInput{
    std::string name;
    size_t free_slots_needed = 1;
    bool menu_opens = true;
    PartySlot last_occupied = PartySlot::ONE;
};

struct PartyCheckExpectation{
    std::string steps = "";
    // Start of the UserSetupError message; empty if the check passes.
    std::string error = "";
};

struct PartyCheckCase{
    PartyCheckInput input;
    PartyCheckExpectation expectation;
};

class Test_PartyCheck : public UnitTest{
public:
    Test_PartyCheck(PartyCheckCase test_case)
        : UnitTest("PokemonFRLG::RngPartyCheck - " + test_case.input.name)
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const PartyCheckInput& input = m_case.input;
        const PartyCheckExpectation& expected = m_case.expectation;
        ScriptedPartyCheck actions;
        actions.menu_opens = input.menu_opens;
        actions.last_occupied = input.last_occupied;

        NullLogger logger;
        std::string error;
        try{
            check_party_free_slots(logger, input.free_slots_needed, actions);
        }catch (UserSetupError& e){
            error = e.message();
        }
        const bool error_matches = expected.error.empty()
            ? error.empty()
            : error.rfind(expected.error, 0) == 0;
        if (!error_matches){
            return std::format("Expected error: \"{}\"\nActual error:   \"{}\"", expected.error, error);
        }
        if (actions.steps != expected.steps){
            return std::format("Expected steps: {}\nActual steps:   {}", expected.steps, actions.steps);
        }
        return true;
    }

private:
    const PartyCheckCase m_case;
};

void add_tests_PartyCheck(UnitTestDatabase& database){
    const PartyCheckCase cases[] = {
        {
            // Programs that don't add a Pokemon to the party don't open the menu.
            .input = {.name = "no free slot needed, full party", .free_slots_needed = 0, .last_occupied = PartySlot::SIX},
            .expectation = {},
        },
        {
            .input = {.name = "one free slot needed, one Pokemon"},
            .expectation = {.steps = "open read"},
        },
        {
            .input = {.name = "one free slot needed, five Pokemon", .last_occupied = PartySlot::FIVE},
            .expectation = {.steps = "open read"},
        },
        {
            .input = {.name = "one free slot needed, full party", .last_occupied = PartySlot::SIX},
            .expectation = {.steps = "open read", .error = "Your party is full. Leave a party slot free"},
        },
        {
            .input = {.name = "three free slots needed, three Pokemon", .free_slots_needed = 3, .last_occupied = PartySlot::THREE},
            .expectation = {.steps = "open read"},
        },
        {
            .input = {.name = "three free slots needed, four Pokemon", .free_slots_needed = 3, .last_occupied = PartySlot::FOUR},
            .expectation = {.steps = "open read", .error = "Your party has 4 "},
        },
        {
            // A menu that can't be opened skips the check rather than stopping the program.
            .input = {.name = "party menu not opened", .menu_opens = false, .last_occupied = PartySlot::SIX},
            .expectation = {.steps = "open"},
        },
    };
    for (const PartyCheckCase& test_case : cases){
        database.add<Test_PartyCheck>(test_case);
    }
}


std::string menu_context_name(StartMenuContext context){
    switch (context){
    case StartMenuContext::STANDARD:    return "STANDARD";
    case StartMenuContext::SAFARI_ZONE: return "SAFARI_ZONE";
    case StartMenuContext::NO_DEX:      return "NO_DEX";
    }
    return "?";
}

template <typename Program>
std::unique_ptr<RngProgramInstance> make_rng_program(){
    return std::make_unique<Program>();
}

struct RngProgramPartyInput{
    std::string name;
    std::function<std::unique_ptr<RngProgramInstance>()> make_program;
    // Program settings by option name, as in the saved settings JSON.
    std::map<std::string, std::string> settings = {};
};

struct RngProgramPartyExpectation{
    size_t free_slots;
    StartMenuContext menu_context = StartMenuContext::STANDARD;
};

struct RngProgramPartyCase{
    RngProgramPartyInput input;
    RngProgramPartyExpectation expectation;
};

// Builds the program with the given settings and checks what its start check asks for. The
// check itself runs for every RNG program: RngProgramInstance's start hook is final.
class Test_RngProgramPartyRequirement : public UnitTest{
public:
    Test_RngProgramPartyRequirement(RngProgramPartyCase test_case)
        : UnitTest("PokemonFRLG::RngProgramPartyRequirement - " + test_case.input.name)
        , m_case(std::move(test_case))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const RngProgramPartyInput& input = m_case.input;
        const RngProgramPartyExpectation& expected = m_case.expectation;
        std::unique_ptr<RngProgramInstance> program = input.make_program();
        JsonObject settings;
        for (const auto& [option, value] : input.settings){
            settings[option] = value;
        }
        program->load_json(JsonValue(std::move(settings)));

        const size_t free_slots = program->free_party_slots_needed();
        if (free_slots != expected.free_slots){
            return std::format("Expected {} free party slots needed, got {}", expected.free_slots, free_slots);
        }
        const StartMenuContext menu_context = program->start_menu_context();
        if (menu_context != expected.menu_context){
            return std::format(
                "Expected start menu {}, got {}", menu_context_name(expected.menu_context), menu_context_name(menu_context)
            );
        }
        return true;
    }

private:
    const RngProgramPartyCase m_case;
};

const RngProgramPartyCase RNG_PROGRAM_PARTY_CASES[] = {
    {
        .input = {.name = "StarterRng", .make_program = make_rng_program<StarterRng>},
        .expectation = {.free_slots = 0},
    },
    {
        // The egg and the two wild Pokemon caught to confirm the seed.
        .input = {.name = "EggRng", .make_program = make_rng_program<EggRng>},
        .expectation = {.free_slots = 3},
    },
    {
        .input = {.name = "GiftRng", .make_program = make_rng_program<GiftRng>},
        .expectation = {.free_slots = 1},
    },
    {
        .input = {.name = "StaticRng", .make_program = make_rng_program<StaticRng>},
        .expectation = {.free_slots = 1},
    },
    {
        .input = {.name = "RoamingLegendaryRng", .make_program = make_rng_program<RoamingLegendaryRng>},
        .expectation = {.free_slots = 1},
    },
    {
        .input = {
            .name = "WildRng on Route 1",
            .make_program = make_rng_program<WildRng>,
            .settings = {{"GAME_LOCATION", "route_1"}},
        },
        .expectation = {.free_slots = 1},
    },
    {
        // The Safari Zone start menu has no POKEMON entry in the usual place.
        .input = {
            .name = "WildRng in the Safari Zone",
            .make_program = make_rng_program<WildRng>,
            .settings = {{"GAME_LOCATION", "safari_zone_entrance"}},
        },
        .expectation = {.free_slots = 1, .menu_context = StartMenuContext::SAFARI_ZONE},
    },
    // RngHelper is the manual tool: whatever the target, it trusts the user's party.
    {
        .input = {.name = "RngHelper lapras", .make_program = make_rng_program<RngHelper>, .settings = {{"TARGET", "lapras"}}},
        .expectation = {.free_slots = 0},
    },
};

void add_tests_RngProgramPartyRequirement(UnitTestDatabase& database){
    for (const RngProgramPartyCase& test_case : RNG_PROGRAM_PARTY_CASES){
        database.add<Test_RngProgramPartyRequirement>(test_case);
    }
}

}


void add_tests(UnitTestDatabase& database){
    add_tests_AdvanceWhiteDialogDetector(database);
    add_tests_AdvanceBattleDialogDetector(database);
    add_tests_BagDetector(database);
    add_tests_BattleLevelUpReader(database);
    add_tests_BattleMenuDetector(database);
    add_tests_BattleMoveSelectionArrow(database);
    add_tests_BoxCellOccupiedDetector(database);
    add_tests_BoxCellReader(database);
    add_tests_BoxGenderReader(database);
    add_tests_BoxSelectionModeDetector(database);
    add_tests_BoxSorter(database);
    add_tests_EliteFourRoomDetector(database);
    add_tests_ExitWildBattle(database);
    add_tests_HallOfFameSavingDetector(database);
    add_tests_LastOccupiedPartySlot(database);
    add_tests_PartyCheck(database);
    add_tests_PartyEmptySlotDetector(database);
    add_tests_PartyHeldItemDetector(database);
    add_tests_PartyLevelUpReader(database);
    add_tests_PartyMenuDetector(database);
    add_tests_PrizeSelectDetector(database);
    add_tests_PartySlotDetector(database);
    add_tests_RngProgramPartyRequirement(database);
    add_tests_StatsReader(database);
    add_tests_SelectionDialogDetector(database);
    add_tests_ShinySymbolDetector(database);
    add_tests_TrainerIdReader(database);
    add_tests_WildEncounterReader(database);
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