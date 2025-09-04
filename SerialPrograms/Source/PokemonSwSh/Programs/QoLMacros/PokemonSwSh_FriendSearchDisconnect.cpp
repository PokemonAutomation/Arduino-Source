/*  Friend Search Disconnect
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_AutoHosts.h"
#include "PokemonSwSh_FriendSearchDisconnect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


FriendSearchDisconnect_Descriptor::FriendSearchDisconnect_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:FriendSearchDisconnect",
        STRING_POKEMON + " SwSh", "Friend Search Disconnect",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/FriendSearchDisconnect.md",
        "Disconnect from the internet using the friend search method.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



FriendSearchDisconnect::FriendSearchDisconnect()
    : USER_SLOT(
        "<b>User Slot:</b><br>Use this profile to disconnect.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 8
    )
{
    PA_ADD_OPTION(USER_SLOT);
}

void FriendSearchDisconnect::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 80ms);
    context.wait_for_all_requests();

    auto snapshot = env.console.video().snapshot();
    if (snapshot){
        HomeMenuWatcher home_menu(env.console);
        int ret = wait_until(
            env.console, context,
            5000ms,
            {home_menu}
        );
        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Home menu not detected after 5 seconds.",
                env.console
            );
        }
    }

    ConsoleType type = env.console.state().console_type();
    uint8_t scroll_down_slots = 0;
    if (is_switch1(type)){
        scroll_down_slots = 1;
    }else if (is_switch2(type)){
        scroll_down_slots = 3;
    }else{
        throw UserSetupError(
            env.logger(),
            "Please select a valid Switch console type."
        );
    }

    home_to_add_friends(context, USER_SLOT - 1, scroll_down_slots, true);

    //  Enter friend search.
    pbf_mash_button(context, BUTTON_A, 2000ms);
    settings_to_enter_game(context, true);
}


}
}
}
