/*  God Egg Item Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_Misc.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonSwSh_GodEggItemDupe.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


GodEggItemDupe_Descriptor::GodEggItemDupe_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:GodEggItemDupe",
        STRING_POKEMON + " SwSh", "God Egg Item Duplication",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/GodEggItemDuplication.md",
        "Mass duplicate items with the God Egg.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



GodEggItemDupe::GodEggItemDupe(const GodEggItemDupe_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        2000
    )
    , PARTY_ROUND_ROBIN(
        "<b>Party Round Robin:</b><br>Cycle through this many party members.",
        6, 1, 6
    )
    , DETACH_BEFORE_RELEASE(
        "<b>Detach before Release:</b><br>Needed for items like Rusted Sword/Shield.",
        false
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(PARTY_ROUND_ROBIN);
    PA_ADD_OPTION(DETACH_BEFORE_RELEASE);
}


void GodEggItemDupe::collect_godegg(
    const BotBaseContext& context,
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
void GodEggItemDupe::run_program(SingleSwitchProgramEnvironment& env, uint16_t attempts) const{
    if (attempts == 0){
        return;
    }

    uint8_t items = PARTY_ROUND_ROBIN > 6 ? 6 : PARTY_ROUND_ROBIN;
    uint8_t party_slot = 0;

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(env.console, true);
        collect_godegg(env.console, party_slot++, true, false);
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
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        eggfetcher_loop(env.console);
        collect_egg(env.console);
        collect_godegg(env.console, party_slot++, false, false);
        if (party_slot >= items){
            party_slot = 0;
        }
    }
}

void GodEggItemDupe::program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    run_program(env, MAX_FETCH_ATTEMPTS);
    ssf_press_button2(env.console, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE, 10);
}




}
}
}
