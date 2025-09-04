/*  God Egg Item Duplication
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
#include "PokemonSwSh/Programs/PokemonSwSh_ReleaseHelpers.h"
#include "PokemonSwSh_GodEggItemDupe.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


GodEggItemDupe_Descriptor::GodEggItemDupe_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:GodEggItemDupe",
        STRING_POKEMON + " SwSh", "God Egg Item Duplication",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/GodEggItemDuplication.md",
        "Mass duplicate items with the God Egg.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



GodEggItemDupe::GodEggItemDupe()
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
    , DETACH_BEFORE_RELEASE(
        "<b>Detach before Release:</b><br>Needed for items like Rusted Sword/Shield.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(PARTY_ROUND_ROBIN);
    PA_ADD_OPTION(DETACH_BEFORE_RELEASE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void GodEggItemDupe::collect_godegg(
    ProControllerContext& context,
    uint8_t party_slot,
    bool map_to_pokemon,
    bool pokemon_to_map
) const{
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

    //  Enter box
    ssf_press_button(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0, 160ms);
    if (map_to_pokemon){
        ssf_press_dpad_ptv(context, DPAD_UP, 160ms, 80ms);
        ssf_press_dpad_ptv(context, DPAD_RIGHT, 160ms, 80ms);
    }
    ssf_press_button(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY0, 80ms);
    ssf_press_button(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY0, 80ms);

    if (DETACH_BEFORE_RELEASE){
        //  Detach item
        ssf_press_button(context, BUTTON_A, 400ms, 160ms);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 80ms);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 80ms);
        ssf_press_button(context, BUTTON_A, 1200ms, 160ms);
        ssf_press_button(context, BUTTON_A, 1200ms, 160ms);
        ssf_press_button(context, BUTTON_A, 800ms, 160ms);

        //  Release
        release(context);
    }else{
        //  Release (item detaches automatically)
        ssf_press_button(context, BUTTON_A, 480ms, 160ms);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 120ms);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 120ms);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 120ms);
        ssf_press_dpad_ptv(context, DPAD_DOWN, 120ms);
        ssf_press_button(context, BUTTON_A, 1000ms, 160ms);
        ssf_press_dpad_ptv(context, DPAD_UP, 80ms);
        ssf_mash_AZs(context, 180);
    }

    //  Back out to menu.
    if (pokemon_to_map){
        box_to_menu(context);
        ssf_press_button(context, BUTTON_B, 2000ms, 50ms);
    }else{
        pbf_mash_button(context, BUTTON_B, 700);
    }
}
void GodEggItemDupe::run_program(Logger& logger, ProControllerContext& context, uint16_t attempts) const{
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
        collect_godegg(context, party_slot++, true, false);
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
        collect_godegg(context, party_slot++, false, false);
        if (party_slot >= items){
            party_slot = 0;
        }
    }
}

void GodEggItemDupe::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
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
