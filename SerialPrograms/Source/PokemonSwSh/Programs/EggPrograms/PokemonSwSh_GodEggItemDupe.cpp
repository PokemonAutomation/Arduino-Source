/*  God Egg Item Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
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
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



GodEggItemDupe::GodEggItemDupe()
    : MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        LockWhileRunning::LOCKED,
        2000
    )
    , PARTY_ROUND_ROBIN(
        "<b>Party Round Robin:</b><br>Cycle through this many party members.",
        LockWhileRunning::LOCKED,
        6, 1, 6
    )
    , DETACH_BEFORE_RELEASE(
        "<b>Detach before Release:</b><br>Needed for items like Rusted Sword/Shield.",
        LockWhileRunning::LOCKED,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(PARTY_ROUND_ROBIN);
    PA_ADD_OPTION(DETACH_BEFORE_RELEASE);
}


void GodEggItemDupe::collect_godegg(
    BotBaseContext& context,
    uint8_t party_slot,
    bool map_to_pokemon,
    bool pokemon_to_map
) const{
    pbf_wait(context, 50);
    ssf_press_button1(context, BUTTON_B, 100);
    ssf_press_button1(context, BUTTON_B, 100);
    pbf_wait(context, 225);

    //  "You received an Egg from the Nursery worker!"
    ssf_press_button1(context, BUTTON_B, 300);

    //  "Where do you want to send the Egg to?"
    ssf_press_button1(context, BUTTON_A, 100);

    //  (extra line of text for French)
    ssf_press_button1(context, BUTTON_B, 100);

    //  "Please select a Pokemon to swap from your party."
    ssf_press_button1(context, BUTTON_B, GameSettings::instance().MENU_TO_POKEMON_DELAY);

    //  Select the party member.
    for (uint8_t c = 0; c < party_slot; c++){
        ssf_press_dpad1(context, DPAD_DOWN, 10);
    }
    ssf_press_button1(context, BUTTON_A, 300);
    pbf_mash_button(context, BUTTON_B, 500);

    //  Enter box
    ssf_press_button2(context, BUTTON_X, GameSettings::instance().OVERWORLD_TO_MENU_DELAY, 20);
    if (map_to_pokemon){
        ssf_press_dpad2(context, DPAD_UP, 20, 10);
        ssf_press_dpad2(context, DPAD_RIGHT, 20, 10);
    }
    ssf_press_button2(context, BUTTON_A, GameSettings::instance().MENU_TO_POKEMON_DELAY, 10);
    ssf_press_button2(context, BUTTON_R, GameSettings::instance().POKEMON_TO_BOX_DELAY, 10);

    if (DETACH_BEFORE_RELEASE){
        //  Detach item
        ssf_press_button2(context, BUTTON_A, 50, 10);
        ssf_press_dpad1(context, DPAD_DOWN, 10);
        ssf_press_dpad1(context, DPAD_DOWN, 10);
        ssf_press_button2(context, BUTTON_A, 150, 10);
        ssf_press_button2(context, BUTTON_A, 150, 10);
        ssf_press_button2(context, BUTTON_A, 100, 10);

        //  Release
        release(context);
    }else{
        //  Release (item detaches automatically)
        ssf_press_button2(context, BUTTON_A, 60, 10);
        ssf_press_dpad1(context, DPAD_DOWN, 15);
        ssf_press_dpad1(context, DPAD_DOWN, 15);
        ssf_press_dpad1(context, DPAD_DOWN, 15);
        ssf_press_dpad1(context, DPAD_DOWN, 15);
        ssf_press_button2(context, BUTTON_A, 125, 10);
        ssf_press_dpad1(context, DPAD_UP, 10);
        mash_A(context, 180);
    }

    //  Back out to menu.
    if (pokemon_to_map){
        box_to_menu(context);
        ssf_press_button1(context, BUTTON_B, 250);
    }else{
        pbf_mash_button(context, BUTTON_B, 700);
    }
}
void GodEggItemDupe::run_program(Logger& logger, BotBaseContext& context, uint16_t attempts) const{
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

void GodEggItemDupe::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    run_program(env.console, context, MAX_FETCH_ATTEMPTS);
    ssf_press_button2(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE, 10);
}




}
}
}
