/*  Game Entry
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Async/InferenceRoutines.h"
#include "PokemonLZA/PokemonLZA_Settings.h"
#include "PokemonLZA_GameEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

bool reset_game_from_home(
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool backup_save,
    Milliseconds post_wait_time
){
    return PokemonLA::reset_game_from_home(
        env, console, context,
        backup_save,
        GameSettings::instance().ENTER_GAME_MASH,
        GameSettings::instance().ENTER_GAME_WAIT,
        post_wait_time
    );
}


}
}
}
