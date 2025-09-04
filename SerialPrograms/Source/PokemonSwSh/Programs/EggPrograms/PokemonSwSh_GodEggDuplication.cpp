/*  God Egg Duplication
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh_GodEggDuplication.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


GodEggDuplication_Descriptor::GodEggDuplication_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:GodEggDuplication",
        STRING_POKEMON + " SwSh", "God Egg Duplication",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/GodEggDuplication.md",
        "Mass duplicate " + STRING_POKEMON + " with the God Egg.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



GodEggDuplication::GodEggDuplication()
    : MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        LockMode::LOCK_WHILE_RUNNING,
        2000
    )
    , PARTY_ROUND_ROBIN(
        "<b>Party Round Robin:</b><br>Cycle through this many party members.",
        LockMode::LOCK_WHILE_RUNNING,
        6, 1, 6
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(PARTY_ROUND_ROBIN);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void GodEggDuplication::collect_godegg(ProControllerContext& context, uint8_t party_slot) const{
    pbf_wait(context, 50);
    ssf_press_button_ptv(context, BUTTON_B, 800ms);
    ssf_press_button_ptv(context, BUTTON_B, 800ms);
    pbf_wait(context, 225);

    //  "You received an Egg from the Nursery worker!"
    ssf_press_button_ptv(context, BUTTON_B, 2400ms);

    //  "Where do you want to send the Egg to?"
    ssf_press_button_ptv(context, BUTTON_A, 800ms);

    //  (extra line of text for French)
    ssf_press_button_ptv(context, BUTTON_B, 800ms);

    //  "Please select a Pokemon to swap from your party."
    ssf_press_button_ptv(context, BUTTON_B, GameSettings::instance().MENU_TO_POKEMON_DELAY0);

    //  Select the party member.
    for (uint8_t c = 0; c < party_slot; c++){
        ssf_press_dpad_ptv(context, DPAD_DOWN, 80ms);
    }
    ssf_press_button_ptv(context, BUTTON_A, 2400ms);
    pbf_mash_button(context, BUTTON_B, 500);
}
void GodEggDuplication::run_program(Logger& logger, ProControllerContext& context, uint16_t attempts) const{
    if (attempts == 0){
        return;
    }

    uint8_t items = PARTY_ROUND_ROBIN > 6 ? 6 : PARTY_ROUND_ROBIN;
    uint8_t party_slot = 0;

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        logger.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(context, true);
        collect_godegg(context, party_slot++);
        if (party_slot >= items){
            party_slot = 0;
        }

        c++;
        if (c >= attempts){
            return;
        }
    }

    //  Now we are in steady state.
    for (; c < attempts; c++){
        logger.log("Fetch Attempts: " + tostr_u_commas(c));
        eggfetcher_loop(context);
        collect_egg(context);
        collect_godegg(context, party_slot++);
        if (party_slot >= items){
            party_slot = 0;
        }
    }
}

void GodEggDuplication::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    run_program(env.console, context, MAX_FETCH_ATTEMPTS);
    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}

