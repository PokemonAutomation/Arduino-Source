/*  Egg Fetcher 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShEggRoutines.h"
#include "PokemonSwSh_EggFetcher2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


EggFetcher2_Descriptor::EggFetcher2_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:EggFetcher2",
        "Egg Fetcher 2",
        "SwSh-Arduino/wiki/Basic:-EggFetcher2",
        "Fetch eggs without hatching them.",
        FeedbackType::NONE,
        PABotBaseLevel::PABOTBASE_31KB
    )
{}



EggFetcher2::EggFetcher2(const EggFetcher2_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        2000
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(MAX_FETCH_ATTEMPTS);
}


void EggFetcher2::run_eggfetcher(
    SingleSwitchProgramEnvironment& env,
    bool deposit_automatically,
    uint16_t attempts
) const{
    if (attempts == 0){
        return;
    }

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(env.console, true);
        collect_egg_mash_out(env.console, deposit_automatically);

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
        collect_egg_mash_out(env.console, deposit_automatically);
    }
}

void EggFetcher2::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    run_eggfetcher(env, AUTO_DEPOSIT, MAX_FETCH_ATTEMPTS);

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback(env.console);
    end_program_loop(env.console);
}


}
}
}
