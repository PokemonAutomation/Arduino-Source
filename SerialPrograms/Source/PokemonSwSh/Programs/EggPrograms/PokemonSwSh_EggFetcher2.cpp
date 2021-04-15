/*  Egg Fetcher 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShEggRoutines.h"
#include "PokemonSwSh_EggFetcher2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

EggFetcher2::EggFetcher2()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_31KB,
        "Egg Fetcher 2",
        "NativePrograms/EggFetcher2.md",
        "Fetch eggs without hatching them."
    )
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        2000
    )
{
    m_options.emplace_back(&MAX_FETCH_ATTEMPTS, "MAX_FETCH_ATTEMPTS");
}


void EggFetcher2::run_eggfetcher(SingleSwitchProgramEnvironment& env, bool deposit_automatically, uint16_t attempts) const{
    if (attempts == 0){
        return;
    }

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(true);
        collect_egg_mash_out(deposit_automatically);

        c++;
        if (c >= attempts){
            return;
        }
    }

    //  Now we are in steady state.
    for (; c < attempts; c++){
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        eggfetcher_loop();
        collect_egg();
        collect_egg_mash_out(deposit_automatically);
    }
}

void EggFetcher2::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);

    run_eggfetcher(env, AUTO_DEPOSIT, MAX_FETCH_ATTEMPTS);

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    end_program_callback();
    end_program_loop();
}


}
}
}
