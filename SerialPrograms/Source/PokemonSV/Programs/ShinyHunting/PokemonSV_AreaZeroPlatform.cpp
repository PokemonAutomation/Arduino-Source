/*  Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.h"
#include "PokemonSV/Programs/PokemonSV_AreaZero.h"
#include "PokemonSV_LetsGoTools.h"
#include "PokemonSV_AreaZeroPlatform.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



PlatformResetSettings::~PlatformResetSettings(){
    ENCOUNTERS_IN_WINDOW.remove_listener(*this);
    KILLS_IN_WINDOW0.remove_listener(*this);
    WINDOW_IN_MINUTES.remove_listener(*this);
}
PlatformResetSettings::PlatformResetSettings()
    : GroupOption(
        "Platform Reset Conditions",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_ENABLED
    )
    , m_description(
        "A \"Platform Reset\" is when you fly to Zero Gate, then return to the "
        "platform. This is usually done to recover from falling off the "
        "platform or to reset the spawns on the platform.<br><br>"
        "The reason for resetting spawns is that over time, the " + STRING_POKEMON +
        " can spawn in inaccessible places. When there are too few spawns, the "
        "kill and encounter rates will drop to very inefficient levels. "
        "Resetting the spawns will fix this, at the cost of also despawning any "
        "shinies that have not yet been encountered."
    )
    , m_sliding_window("")
    , WINDOW_IN_MINUTES(
        "<b>Time Window (in minutes):</b><br>The sliding time window for which to watch for kills and encounters.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10
    )
    , KILLS_IN_WINDOW0(
        "<b>Kills in Window:</b><br>If the number of kills in the last X seconds has drops below this value, consider resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        20
    )
    , ENCOUNTERS_IN_WINDOW(
        "<b>Encounters in Window:</b><br>If the number of encounters in the last X seconds has drops below this value, consider resetting.",
        LockMode::UNLOCK_WHILE_RUNNING,
        5
    )
#if 0
    , RESET_DURATION_MINUTES(
        "<b>Reset Duration (minutes):</b><br>If you are resetting, reset the game every "
        "this many minutes.",
        LockMode::UNLOCK_WHILE_RUNNING,
        180
    )
#endif
{
    PA_ADD_STATIC(m_description);

    PA_ADD_STATIC(m_sliding_window);
    PA_ADD_OPTION(WINDOW_IN_MINUTES);
    PA_ADD_OPTION(KILLS_IN_WINDOW0);
    PA_ADD_OPTION(ENCOUNTERS_IN_WINDOW);

//    PA_ADD_OPTION(RESET_DURATION_MINUTES);

    PlatformResetSettings::on_config_value_changed(this);

    WINDOW_IN_MINUTES.add_listener(*this);
    KILLS_IN_WINDOW0.add_listener(*this);
    ENCOUNTERS_IN_WINDOW.add_listener(*this);
}

std::string int_to_text(size_t value, const std::string& unit){
    std::string str = "<b><em>" + std::to_string(value) + "</em></b> ";
    str += unit;
    if (value != 1){
        str += "s";
    }
    return str;
}
void PlatformResetSettings::on_config_value_changed(void* object){
    m_sliding_window.set_text(
        "<font color=\"red\">Perform a platform reset if there are fewer than " + int_to_text(KILLS_IN_WINDOW0, "kill") +
        " and " + int_to_text(ENCOUNTERS_IN_WINDOW, "encounter") +
        " in the last " + int_to_text(WINDOW_IN_MINUTES, "minute") + ".</font>"
    );
}


NavigatePlatformSettings::NavigatePlatformSettings()
    : GroupOption("Navigate to Platform Settings", LockMode::UNLOCK_WHILE_RUNNING)
    , m_description(
        "These settings are used when traveling from Zero Gate to the platform. "
        "This can happen either at program start or during a platform reset."
    )
    , HEAL_AT_STATION(
        "<b>Heal at Station:</b><br>If you're passing through the station, take the opportunity to heal up.",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , STATION_ARRIVE_PAUSE_SECONDS(
        "<b>Station Arrive Pause Time:</b><br>Pause for this many seconds after leaving the station. "
        "This gives the game time to load and thus reduce the chance of lag affecting the flight path.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1
    )
    , MIDAIR_PAUSE_TIME0(
        "<b>Mid-Air Pause Time:</b><br>Pause for this long before final approach to the platform. "
        "Too small and you may crash into the wall above the platform or have reduced spawns. "
        "Too large and you may undershoot the platform.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "400 ms"
    )
{
    PA_ADD_STATIC(m_description);
    PA_ADD_OPTION(HEAL_AT_STATION);
    PA_ADD_OPTION(STATION_ARRIVE_PAUSE_SECONDS);
    PA_ADD_OPTION(MIDAIR_PAUSE_TIME0);
}


void inside_zero_gate_to_platform(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    bool flying_unlocked,
    NavigatePlatformSettings& settings
){
    inside_zero_gate_to_station(info, stream, context, 2, settings.HEAL_AT_STATION);

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

    ssf_press_button(context, BUTTON_LCLICK, 0ms, 4000ms);
    if (!flying_unlocked){
        ssf_press_left_joystick(context, 128, 0, 125, 1250);
    }else{
        ssf_press_left_joystick(context, 128, 0, 125, 875);
    }

    //  Jump
    ssf_press_button(context, BUTTON_B, 1000ms, 800ms);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0ms, 160ms, 80ms);  //  Double up this press in
    ssf_press_button(context, BUTTON_B, 0ms, 160ms);        //  case one is dropped.

    if (!flying_unlocked){
//        ssf_press_left_joystick(context, 128, 0, 375, 875);
        pbf_move_left_joystick(context, 144, 0, 700, 0);
        pbf_move_left_joystick(context, 128, 0, 1000ms, settings.MIDAIR_PAUSE_TIME0);
        pbf_move_left_joystick(context, 128, 0, 875, 250);
    }else{
//        ssf_press_button(context, BUTTON_B, 0, 20);
//        pbf_move_left_joystick(context, 128, 0, 375, 250);
        pbf_move_left_joystick(context, 164, 0, 1000ms, settings.MIDAIR_PAUSE_TIME0);
        pbf_press_button(context, BUTTON_LCLICK, 50, 0);
        ssf_press_right_joystick(context, 128, 255, 0, 1500);
        pbf_move_left_joystick(context, 128, 255, 1600, 125);

        pbf_press_button(context, BUTTON_B, 125, 375);

    }
#endif

//    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    ssf_press_left_joystick(context, 128, 0, 1 * TICKS_PER_SECOND, 4 * TICKS_PER_SECOND);
    pbf_press_button(context, BUTTON_R, 20, 355);
    pbf_press_button(context, BUTTON_R, 20, 105);

    context.wait_for_all_requests();
}


bool read_platform_center(
    double& x, double& y,
    const ProgramInfo& info, VideoStream& stream
){
    using namespace Kernels::Waterfill;

    VideoSnapshot screen = stream.video().snapshot();
    if (!screen){
        return false;
    }

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(screen, 0xff000040, 0xff8080ff);

//    size_t min_width = screen.width() / 4;
//    size_t min_height = screen.height() / 4;

    WaterfillObject biggest;
    WaterfillObject object;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    size_t min_area = screen->width() * screen->height() / 10;
    auto iter = session->make_iterator(min_area);
    while (iter->find_next(object, false)){
//        if (object.min_y != 0){
//            continue;
//        }
        if (biggest.area < object.area){
            biggest = std::move(object);
        }
    }

    if (biggest.area == 0){
        return false;
    }

    x = biggest.center_of_gravity_x() / screen->width();
    y = biggest.center_of_gravity_y() / screen->height();

    return true;
}



void area_zero_platform_run_path0(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    LetsGoEncounterBotTracker& tracker,
    uint64_t iteration_count
){
    //  Go back to the wall.
    stream.log("Go back to wall...");
    use_lets_go_to_clear_in_front(stream, context, tracker, false, [&](ProControllerContext& context){
        find_and_center_on_sky(env, stream, context);
        pbf_move_right_joystick(context, 128, 255, 80, 0);
        pbf_move_left_joystick(context, 176, 255, 30, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    use_lets_go_to_clear_in_front(stream, context, tracker, false, [&](ProControllerContext& context){
        //  Move to wall.
        pbf_move_left_joystick(context, 128, 0, 4 * TICKS_PER_SECOND, 0);

        //  Turn around.
        stream.log("Turning towards sky...");
        pbf_move_left_joystick(context, 128, 255, 30, 95);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    //  Move forward and kill everything in your path.
    stream.log("Moving towards sky and killing everything...");
    uint16_t duration = 325;
    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
        find_and_center_on_sky(env, stream, context);
        pbf_move_right_joystick(context, 128, 255, 70, 0);

        uint8_t x = 128;
        switch (iteration_count % 4){
        case 0:
            x = 96;
            duration = 250;
            break;
        case 1:
            x = 112;
            break;
        case 2:
            x = 128;
            break;
        case 3:
            x = 112;
            break;
        }

        ssf_press_button(context, BUTTON_L, 0ms, 160ms);
        pbf_move_left_joystick(context, x, 0, duration, 0);
    });
    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
        pbf_move_left_joystick(context, 128, 255, duration, 4 * TICKS_PER_SECOND);
    });
}
void area_zero_platform_run_path1(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    LetsGoEncounterBotTracker& tracker,
    uint64_t iteration_count
){
    //  Go back to the wall.
    stream.log("Go back to wall...");
    pbf_press_button(context, BUTTON_L, 20, 105);
    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
        find_and_center_on_sky(env, stream, context);
        pbf_move_right_joystick(context, 128, 255, 80, 0);
        pbf_move_left_joystick(context, 192, 255, 60, 0);
    });

    //  Clear path to the wall.
    stream.log("Clear path to the wall...");
    pbf_press_button(context, BUTTON_L, 20, 50);
    use_lets_go_to_clear_in_front(stream, context, tracker, false, [&](ProControllerContext& context){
        pbf_move_left_joystick(context, 128, 0, 5 * TICKS_PER_SECOND, 0);

        //  Turn right.
        pbf_move_left_joystick(context, 255, 128, 30, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
    });

    //  Clear the wall.
    stream.log("Clear the wall...");
    uint16_t duration = 325;
    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
        pbf_move_left_joystick(context, 255, 128, 125, 0);
        pbf_press_button(context, BUTTON_L, 20, 50);
        context.wait_for_all_requests();

        find_and_center_on_sky(env, stream, context);
        pbf_move_left_joystick(context, 128, 0, 50, 0);
        pbf_press_button(context, BUTTON_L, 20, 30);

        //  Move forward.

        uint8_t x = 128;
        switch (iteration_count % 4){
        case 0:
            x = 96;
            duration = 250;
            break;
        case 1:
            x = 112;
            break;
        case 2:
            x = 128;
            break;
        case 3:
            x = 112;
            break;
        }

        pbf_move_left_joystick(context, x, 0, duration, 0);
    });

    stream.log("Run backwards and wait...");
    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
//        pbf_move_left_joystick(context, 64, 0, 125, 0);
//        pbf_press_button(context, BUTTON_L, 20, 105);
        pbf_move_left_joystick(context, 128, 255, duration, 4 * TICKS_PER_SECOND);
//        pbf_controller_state(context, 0, DPAD_NONE, 255, 255, 120, 128, 3 * TICKS_PER_SECOND);
    });
}


void direction_to_stick(
    uint8_t& joystick_x, uint8_t& joystick_y,
    double direction_x, double direction_y
){
//    cout << "direction = " << direction_x << ", " << direction_y << endl;

    double scale = std::max(std::abs(direction_x), std::abs(direction_y));
    direction_x = 128 / scale * direction_x + 128;
    direction_y = 128 / scale * direction_y + 128;

//    cout << "joystick = " << direction_x << ", " << direction_y << endl;

    direction_x = std::min(direction_x, 255.);
    direction_x = std::max(direction_x, 0.);
    direction_y = std::min(direction_y, 255.);
    direction_y = std::max(direction_y, 0.);

    joystick_x = (uint8_t)direction_x;
    joystick_y = (uint8_t)direction_y;
}
void choose_path(
    Logger& logger,
    uint8_t& x, uint8_t& y, uint16_t& duration,
    double platform_x, double platform_y
){
    double diff_x = platform_x - 0.62;
    double diff_y = platform_y - 0.71;

    logger.log("Move Direction: x = " + tostr_default(diff_x) + ", y = " + tostr_default(diff_y), COLOR_BLUE);

    direction_to_stick(x, y, diff_x, diff_y);
    duration = (uint16_t)std::min<double>(std::sqrt(diff_x*diff_x + diff_y*diff_y) * 125 * 12, 400);
}
void turn_angle(ProControllerContext& context, double angle_radians){
    uint8_t turn_x, turn_y;
    direction_to_stick(turn_x, turn_y, -std::sin(angle_radians), std::cos(angle_radians));
    pbf_move_left_joystick(context, turn_x, turn_y, 40, 20);
    pbf_mash_button(context, BUTTON_L, 60);
}

void area_zero_platform_run_path2(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    LetsGoEncounterBotTracker& tracker,
    uint64_t iteration_count
){
    stream.log("Look forward and fire...");
    pbf_mash_button(context, BUTTON_L, 60);

    double platform_x, platform_y;
    uint16_t duration;
    uint8_t move_x, move_y;
    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){

        stream.log("Find the sky, turn around and fire.");
        pbf_move_right_joystick(context, 128, 0, 60, 0);
        find_and_center_on_sky(env, stream, context);
        context.wait_for(std::chrono::seconds(1));
        pbf_move_left_joystick(context, 128, 255, 40, 85);
        pbf_mash_button(context, BUTTON_L, 60);

        pbf_move_right_joystick(context, 128, 255, 250, 0);
        context.wait_for_all_requests();

        stream.log("Finding center of platform...");
        if (!read_platform_center(platform_x, platform_y, env.program_info(), stream)){
            stream.log("Unable to find center of platform.", COLOR_RED);
            return;
        }
        stream.log("Platform center at: x = " + tostr_default(platform_x) + ", y = " + tostr_default(platform_y), COLOR_BLUE);

        choose_path(stream.logger(), move_x, move_y, duration, platform_x, platform_y);

        pbf_move_left_joystick(context, move_x, move_y, 40, 20);
        pbf_mash_button(context, BUTTON_L, 60);
//        pbf_wait(context, 1250);
    });
    use_lets_go_to_clear_in_front(stream, context, tracker, duration > 100, [&](ProControllerContext& context){
        stream.log("Making location correction...");
        pbf_move_left_joystick(context, 128, 0, duration, 0);

        //  Optimization, calculate angle to aim you back at the sky.
        //  This speeds up the "find_and_center_on_sky()" call.
        double angle0 = std::atan2(move_x - 128., 128. - move_y);
        double angle1 = angle0 >= 0 ? 6.2831853071795864769 - angle0 : -6.2831853071795864769 - angle0;
        turn_angle(context, angle1);

        find_and_center_on_sky(env, stream, context);
        pbf_move_left_joystick(context, 96, 0, 40, 0);
        pbf_mash_button(context, BUTTON_L, 60);
    });

    //  One in every 4 iterations: Clear wall of spawns.
    if (iteration_count % 4 == 0){
        stream.log("Turning along wall...");
        pbf_move_left_joystick(context, 0, 255, 20, 20);
        pbf_mash_button(context, BUTTON_L, 60);
        use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
            context.wait_for(std::chrono::milliseconds(1000));

            stream.log("Turning back to sky.");
            pbf_move_left_joystick(context, 255, 255, 20, 20);
            pbf_mash_button(context, BUTTON_L, 60);
            find_and_center_on_sky(env, stream, context);
            pbf_move_left_joystick(context, 96, 0, 40, 0);
            pbf_mash_button(context, BUTTON_L, 60);
        });
    }

    if (platform_x < 0.5 || platform_y < 0.5){
        stream.log("Not close enough to desired spot. Skipping forward attack...", COLOR_ORANGE);
        return;
    }

    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
        stream.log("Move forward, fire, and retreat.");
        switch (iteration_count % 3){
        case 0:
            pbf_move_left_joystick(context, 108, 0, 300, 0);
            break;
        case 1:
            pbf_move_left_joystick(context, 128, 0, 300, 0);
            break;
        case 2:
            pbf_move_left_joystick(context, 144, 0, 300, 0);
            break;
        }
    });

    use_lets_go_to_clear_in_front(stream, context, tracker, true, [&](ProControllerContext& context){
        pbf_move_left_joystick(context, 128, 255, 4 * TICKS_PER_SECOND, 0);
        pbf_move_left_joystick(context, 128, 0, 60, 4 * TICKS_PER_SECOND);
    });
}







}
}
}
