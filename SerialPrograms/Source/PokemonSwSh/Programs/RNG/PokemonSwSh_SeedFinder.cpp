/*  RNG Manipulation SeedFinder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_SeedFinder.h"

#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;

SeedFinder_Descriptor::SeedFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:SeedFinder",
        STRING_POKEMON + " SwSh", "Seed Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/SeedFinder.md",
        "Finds the current state to be used for manual RNG manipulation.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

SeedFinder::SeedFinder()
    : STATE_0(
        false,
        "<b>state[0]:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        ""
    )
    , STATE_1(
        false,
        "<b>state[1]:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        ""
    )
    , UPDATE_STATE(
        "<b>Update State</b>:<br>Use the last known state to update the rng state.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , MIN_ADVANCES(
        "<b>Min Advances:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , MAX_ADVANCES(
        "<b>Max Advances:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        10000
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SAVE_SCREENSHOTS(
        "<b>Save Debug Screenshots:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , LOG_VALUES(
        "<b>Log Animation Values:</br>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )

{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(STATE_0);
    PA_ADD_OPTION(STATE_1);
    PA_ADD_OPTION(UPDATE_STATE);
    PA_ADD_OPTION(MIN_ADVANCES);
    PA_ADD_OPTION(MAX_ADVANCES);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SAVE_SCREENSHOTS);
    PA_ADD_OPTION(LOG_VALUES);
}



void SeedFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    Xoroshiro128PlusState state(0, 0);
    // sanitize STATE_0 and STATE_1 and make ints
    if (UPDATE_STATE){
        try{
            state.s0 = std::stoull(STATE_0, nullptr, 16);
            state.s1 = std::stoull(STATE_1, nullptr, 16);
        }catch (std::invalid_argument&){
            throw UserSetupError(env.console, "State is invalid.");
        }catch (std::out_of_range&){
            throw UserSetupError(env.console, "State is invalid. Are there any extra characters?");
        }

        // clean up STATE_0 and STATE_1
        env.console.log("Known state: " + tostr_hex(state.s0));
        env.console.log("Known state: " + tostr_hex(state.s1));
        STATE_0.set(tostr_hex(state.s0));
        STATE_1.set(tostr_hex(state.s1));
    }


    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_dpad(context, DPAD_LEFT, 5, 5);
    }


    if (UPDATE_STATE){
        state = refind_rng_state(env.console, context, state, MIN_ADVANCES, MAX_ADVANCES, SAVE_SCREENSHOTS, LOG_VALUES);
    }else{
        state = find_rng_state(env.console, context, SAVE_SCREENSHOTS, LOG_VALUES);
    }

    // show state to the user
    STATE_0.set(tostr_hex(state.s0));
    STATE_1.set(tostr_hex(state.s1));
}


}
}
}
