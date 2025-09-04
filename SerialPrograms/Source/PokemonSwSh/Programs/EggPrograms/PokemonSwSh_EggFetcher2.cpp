/*  Egg Fetcher 2
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
#include "PokemonSwSh_EggFetcher2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


EggFetcher2_Descriptor::EggFetcher2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:EggFetcher2",
        STRING_POKEMON + " SwSh", "Egg Fetcher 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/EggFetcher2.md",
        "Fetch eggs without hatching them.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



EggFetcher2::EggFetcher2()
    : MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        LockMode::LOCK_WHILE_RUNNING,
        2000
    )
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void EggFetcher2::run_eggfetcher(
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

void EggFetcher2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    run_eggfetcher(env.console, context, GameSettings::instance().AUTO_DEPOSIT, MAX_FETCH_ATTEMPTS);

    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
