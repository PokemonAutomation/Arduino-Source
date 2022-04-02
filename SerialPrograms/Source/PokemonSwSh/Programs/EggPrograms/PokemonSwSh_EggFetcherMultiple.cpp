/*  Egg Fetcher Multiple
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_EggRoutines.h"
#include "PokemonSwSh_EggFetcherMultiple.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


std::tuple<uint16_t, uint16_t, uint16_t> get_location(uint16_t index){
    uint16_t box = index / 30;
    index = index % 30;

    uint16_t row = index / 6;
    index = index % 6;

    uint16_t column = index;
    return { box, row, column };
}

EggFetcherMultiple_Descriptor::EggFetcherMultiple_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:EggFetcherMultiple",
        STRING_POKEMON + " SwSh", "Egg Fetcher Multiple",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggFetcherMultiple.md",
        "Fetch eggs from multiple species without hatching them.",
        FeedbackType::NONE, false,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



EggFetcherMultiple::EggFetcherMultiple(const EggFetcherMultiple_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , POKEMON_SPECIES_COUNT(
        "<b>Breed this many species:</b>",
        30
    )
    , MAX_FETCH_ATTEMPTS_PER_SPECIES(
        "<b>Fetch this many times per species:</b>",
        10
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(POKEMON_SPECIES_COUNT);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS_PER_SPECIES);
}


void EggFetcherMultiple::run_eggfetcher(
    SingleSwitchProgramEnvironment& env,
    bool deposit_automatically,
    uint16_t attempts
) const {
    if (attempts == 0) {
        return;
    }

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(env.console, true);
        collect_egg_mash_out(env.console, deposit_automatically);

        c++;
        if (c >= attempts) {
            return;
        }
    }

    //  Now we are in steady state.
    for (; c < attempts; c++) {
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        eggfetcher_loop(env.console);
        collect_egg(env.console);
        collect_egg_mash_out(env.console, deposit_automatically);
    }
}

void EggFetcherMultiple::program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    for (uint16_t s = 0; s < POKEMON_SPECIES_COUNT; ++s){
        fly_home_collect_egg(env.console, true);

        //remove the second pokemon from the daycare
        pbf_press_button(env.console, BUTTON_A , 10, 1.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A , 10, 1.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A , 10, 1 * TICKS_PER_SECOND);
        pbf_press_dpad  (env.console, DPAD_DOWN, 10, 1 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A , 10, 1 * TICKS_PER_SECOND);
        pbf_mash_button (env.console, BUTTON_B , 5 * TICKS_PER_SECOND);

        //add a new second pokemon from the daycare
        pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 1.5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
        auto [box, row, column] = get_location(s);
        if (row == 0 && column == 0 && box != 0){
            box = 1;
        }
        else{
            box = 0;
        }
        for (uint16_t b = 0; b < box; ++b){
            pbf_press_button(env.console, BUTTON_R, 10, 1 * TICKS_PER_SECOND);
        }
        for (uint16_t r = 0; r < row; ++r){
            pbf_press_dpad(env.console, DPAD_DOWN, 10, 1 * TICKS_PER_SECOND);
        }
        for (uint16_t c = 0; c < column; ++c){
            pbf_press_dpad(env.console, DPAD_RIGHT, 10, 1 * TICKS_PER_SECOND);
        }
        pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 5 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);
        pbf_press_button(env.console, BUTTON_A, 10, 1 * TICKS_PER_SECOND);

        run_eggfetcher(env, GameSettings::instance().AUTO_DEPOSIT, MAX_FETCH_ATTEMPTS_PER_SPECIES);
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
}


}
}
}
