/*  Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_AreaZeroPlatform.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



NavigatePlatformSettings::NavigatePlatformSettings()
    : GroupOption("Navigate to Platform Settings", LockWhileRunning::UNLOCKED)
    , STATION_ARRIVE_PAUSE_SECONDS(
        "<b>Station Arrive Pause</b><br>Pause for this many seconds after leaving the station. "
        "This allows stuff to load to reduce the chance of lag affecting the fly to platform.",
        LockWhileRunning::UNLOCKED,
        1
    )
    , MIDAIR_PAUSE_TIME(
        "<b>Mid-Air Pause Time:</b><br>Pause for this long before final approach to the platform. "
        "Too small and you may crash into the wall or have reduced spawns. "
        "Too large and you may undershoot the platform.",
        LockWhileRunning::UNLOCKED,
        TICKS_PER_SECOND,
        "50"
    )
{
    PA_ADD_OPTION(STATION_ARRIVE_PAUSE_SECONDS);
    PA_ADD_OPTION(MIDAIR_PAUSE_TIME);
}


void zero_gate_to_platform(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    NavigatePlatformSettings& settings
){
    zero_gate_to_station(info, console, context, 2);

    context.wait_for(std::chrono::seconds(settings.STATION_ARRIVE_PAUSE_SECONDS));

    //  Navigate to platform.
#if 0
    //  Don't jump to avoid spawn.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 625, 0);
    ssf_press_button(context, BUTTON_B, 0, 50);
    pbf_move_left_joystick(context, 128, 0, 250, 0);
    pbf_move_left_joystick(context, 160, 0, 600, 0);
    pbf_move_left_joystick(context, 128, 0, 1875, 0);
#endif

#if 0
    //  Jump late.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);

    ssf_press_joystick(context, true, 128, 0, 315, 500);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 1150, 0);
    pbf_move_left_joystick(context, 128, 0, 125, NAVIGATE_TO_PLATFORM.MIDAIR_PAUSE_TIME);

    pbf_move_left_joystick(context, 128, 0, 1375, 250);
#endif

#if 0
    //  Jump earlier.
    pbf_press_button(context, BUTTON_PLUS, 20, 105);

    ssf_press_joystick(context, true, 128, 0, 280, 500);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 1150, 0);
    pbf_move_left_joystick(context, 128, 0, 125, NAVIGATE_TO_PLATFORM.MIDAIR_PAUSE_TIME);

    pbf_move_left_joystick(context, 128, 0, 1375, 250);
#endif

#if 1
    //  Jump on the downhill to improve chance of clearing things.
    pbf_move_left_joystick(context, 192, 0, 20, 105);
    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

    ssf_press_button(context, BUTTON_LCLICK, 0, 500);
    ssf_press_joystick(context, true, 128, 0, 125, 1250);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 700, 0);
    pbf_move_left_joystick(context, 128, 0, 125, settings.MIDAIR_PAUSE_TIME);
    pbf_move_left_joystick(context, 128, 0, 875, 250);
#endif

//    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 5 * TICKS_PER_SECOND, 0);
}






}
}
}
