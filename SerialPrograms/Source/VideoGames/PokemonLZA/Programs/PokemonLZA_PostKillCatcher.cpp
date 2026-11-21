/*  Post-Kill Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonLZA/Programs/PokemonLZA_BallThrow.h"
#include "VideoGames/PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA_PostKillCatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

PostKillCatcher_Descriptor::PostKillCatcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:PostKillCatcher",
        STRING_POKEMON + " LZA", "Post-Kill Catcher",
        "Programs/PokemonLZA/PostKillCatcher.html",
        "Save the game immediately after knocking out a " + STRING_POKEMON + ". "
        "Then use this program to repeatedly try to catch it by throwing a ball and resetting.",
        PanelQualityStanding::OFFICIAL,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

PostKillCatcher::PostKillCatcher()
    : RIGHT_SCROLLS(
        "<b>" + STRING_POKEBALL + " Right-Scrolls:</b><br>"
        "Scroll this many balls to the right. Negative will scroll to the left.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0, -15, 15
    )
    , SCROLL_HOLD(
        "<b>Scroll Hold:</b><br>"
        "When scrolling to the desired ball, hold the dpad button for this long.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "120 ms"
    )
    , SCROLL_RELEASE(
        "<b>Scroll Release:</b><br>"
        "When scrolling to the desired ball, release dpad button for this long before the next scroll.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "80 ms"
    )
    , POST_THROW_WAIT(
        "<b>Post-Throw Wait:</b><br>Wait this long after throwing a ball before resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "6000 ms"
    )
{
    PA_ADD_OPTION(RIGHT_SCROLLS);
    PA_ADD_OPTION(SCROLL_HOLD);
    PA_ADD_OPTION(SCROLL_RELEASE);
    PA_ADD_OPTION(POST_THROW_WAIT);
}

void PostKillCatcher::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    require_player(env.console, context, BUTTON_L);

    while (true){
        throw_ball(context, BallThrowOptions{
            .right_scrolls = RIGHT_SCROLLS,
            .scroll_hold = SCROLL_HOLD,
            .scroll_release = SCROLL_RELEASE,
            .after_throw = POST_THROW_WAIT,
        });

        go_home(env.console, context);
        reset_game_from_home(env, env.console, context);

    }

}



}
}
}
