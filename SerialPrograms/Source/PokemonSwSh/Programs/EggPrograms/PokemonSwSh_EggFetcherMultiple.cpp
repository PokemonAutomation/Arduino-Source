/*  Egg Fetcher Multiple
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
#include "PokemonSwSh_EggHelpers.h"
#include "PokemonSwSh_EggFetcherMultiple.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


std::tuple<uint16_t, uint16_t, uint16_t> get_location(uint16_t index){
    uint16_t box = index / 30;
    index = index % 30;

    uint16_t row = index / 6;
    index = index % 6;

    uint16_t column = index;
    return { box, row, column };
}

EggFetcherMultiple_Descriptor::EggFetcherMultiple_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggFetcherMultiple",
        STRING_POKEMON + " SwSh", "Egg Fetcher Multiple",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggFetcherMultiple.md",
        "Fetch eggs from multiple species without hatching them.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



EggFetcherMultiple::EggFetcherMultiple()
    : POKEMON_SPECIES_COUNT(
        "<b>Breed this many species:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        30
    )
    , MAX_FETCH_ATTEMPTS_PER_SPECIES(
        "<b>Fetch this many times per species:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        10
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(POKEMON_SPECIES_COUNT);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS_PER_SPECIES);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void EggFetcherMultiple::run_eggfetcher(
    Logger& logger, ProControllerContext& context,
    bool deposit_automatically,
    uint16_t attempts
) const{
    if (attempts == 0){
        return;
    }

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        logger.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(context, true);
        collect_egg_mash_out(context, deposit_automatically);

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
        collect_egg_mash_out(context, deposit_automatically);
    }
}

void EggFetcherMultiple::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    for (uint16_t s = 0; s < POKEMON_SPECIES_COUNT; ++s){
        fly_home_collect_egg(context, true);

        //remove the second pokemon from the daycare
        pbf_press_button(context, BUTTON_A , 160ms, 1500ms);
        pbf_press_button(context, BUTTON_A , 160ms, 1500ms);
        pbf_press_button(context, BUTTON_A , 160ms, 1000ms);
        pbf_press_dpad  (context, DPAD_DOWN, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_A , 160ms, 1000ms);
        pbf_mash_button (context, BUTTON_B , 5000ms);

        //add a new second pokemon from the daycare
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 1500ms);
        pbf_press_button(context, BUTTON_A, 160ms, 2000ms);
        auto [box, row, column] = get_location(s);
        if (row == 0 && column == 0 && box != 0){
            box = 1;
        }else{
            box = 0;
        }
        for (uint16_t b = 0; b < box; ++b){
            pbf_press_button(context, BUTTON_R, 160ms, 1000ms);
        }
        for (uint16_t r = 0; r < row; ++r){
            pbf_press_dpad(context, DPAD_DOWN, 160ms, 1000ms);
        }
        for (uint16_t c = 0; c < column; ++c){
            pbf_press_dpad(context, DPAD_RIGHT, 160ms, 1000ms);
        }
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 2000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 5000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 160ms, 1000ms);

        run_eggfetcher(env.console, context, GameSettings::instance().AUTO_DEPOSIT, MAX_FETCH_ATTEMPTS_PER_SPECIES);
    }

    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
