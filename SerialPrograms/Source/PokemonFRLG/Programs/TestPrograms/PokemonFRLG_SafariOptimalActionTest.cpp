/*  Safari Optimal Action Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG/Programs/PokemonFRLG_BattleMenuNavigation.h"
#include "PokemonFRLG/Programs/PokemonFRLG_SafariOptimalAction.h"
#include "PokemonFRLG_SafariOptimalActionTest.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace std::chrono_literals;

namespace{

std::string safari_action_to_string(SafariBattleMenuOption action){
    switch (action){
    case SafariBattleMenuOption::BALL:
        return "BALL";
    case SafariBattleMenuOption::BAIT:
        return "BAIT";
    case SafariBattleMenuOption::ROCK:
        return "ROCK";
    case SafariBattleMenuOption::RUN:
        return "RUN";
    default:
        return "UNKNOWN";
    }
}

std::string safari_actions_to_string(const std::vector<SafariBattleMenuOption>& actions){
    std::string ret;
    for (size_t c = 0; c < actions.size(); c++){
        if (!ret.empty()){
            ret += " -> ";
        }
        ret += safari_action_to_string(actions[c]);
    }
    return ret;
}

}

SafariOptimalActionTest_Descriptor::SafariOptimalActionTest_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:SafariOptimalActionTest",
        Pokemon::STRING_POKEMON + " FRLG",
        "Safari Optimal Action Test", "",
        "Start in a Safari Zone encounter with the Safari battle menu already visible. "
        "The program reads the encounter and attempts to execute the optimal actions until the Safari battle menu is no longer detected.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    ){}

SafariOptimalActionTest::SafariOptimalActionTest()
    : LANGUAGE(
        "<b>Game Language:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING, true
    )
    , BALLS_REMAINING(
        "<b>Safari Balls Remaining:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        (uint8_t)30, (uint8_t)1, (uint8_t)30
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BALLS_REMAINING);
}

void SafariOptimalActionTest::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading encounter...");
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonFRLG_WildEncounter encounter = reader.read_encounter(env.logger(), LANGUAGE, screen, SAFARI_ZONE_POKEMON_SUBSET);
    env.log("Encounter: " + encounter.name);

    SafariOptimalAction safari_optimal_action(LANGUAGE);
    auto actions = safari_optimal_action.get_optimal_actions(
        env.console,
        encounter.name,
        BALLS_REMAINING
    );

    if (!actions.has_value()){
        env.log("No optimal action sequence found for this encounter.", COLOR_RED);
        return;
    }

    const std::vector<SafariBattleMenuOption>& action_list = actions->get();

    env.log("Optimal actions: " + safari_actions_to_string(action_list));

    BattleSelectionArrowWatcher battle_arrow(COLOR_RED, &env.console.overlay());

    for (size_t c = 0; c < action_list.size(); c++){
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(10),
            { battle_arrow }
        );

        if (ret != 0){
            env.log("Safari battle menu not detected. Stopping.", COLOR_RED);
            return;
        }

        SafariBattleMenuOption action = action_list[c];
        env.log(
            "Executing action " + std::to_string(c + 1) + "/" + std::to_string(action_list.size()) + ": " + safari_action_to_string(action)
        );

        if (!move_cursor_to_option(env.console, context, action)){
            env.log("Failed to move cursor to option.", COLOR_RED);
            return;
        }

        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        context.wait_for_all_requests();
    }

    env.log("Finished executing Safari optimal actions.", COLOR_BLUE);
}

}
}
}
