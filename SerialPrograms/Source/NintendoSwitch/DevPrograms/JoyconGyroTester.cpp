/*  Joycon Gyro Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"
#include "JoyconGyroTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


JoyconGyroTester_Descriptor::JoyconGyroTester_Descriptor()
    : SingleSwitchProgramDescriptor(
        "NintendoSwitch:JoyconGyroTester",
        "Nintendo Switch", "Joycon Gyro Tester",
        "",
        "Send test values to the Joycon accelerometer and gyroscope axes.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

JoyconGyroTester::JoyconGyroTester()
{

}

JoyconGyroTester::~JoyconGyroTester() = default;

void JoyconGyroTester::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    using namespace std::chrono_literals;
    JoyconController& controller = env.console.controller<JoyconController>();

    controller.wait_for_all(&scope);

    // Enter the throw stance on every run so the game recenters the ball before
    // it receives gyro motion. A short pause is enough for the stance animation.
    controller.issue_buttons(&scope, 0ms, 64ms, 0ms, BUTTON_A);
    controller.wait_for_all(&scope);
    controller.issue_nop(&scope, 400ms);

    // These values are in the canonical, controller-independent frame. This
    // tester uses a right Joy-Con, so the old raw Y/Z values are negated here;
    // the motion API converts them back to that controller's report basis.
    const GyroState neutral{};
    const GyroState wind_up{
        {-5.0, -0.3, 0.8},
        {-168.005384787974, -8.400269239399, -8.400269239399},
    };
    const GyroState release{
        {8.0, -1.5, -4.0},
        {630.020192954902, 35.001121830828, -42.001346196993},
    };
    const GyroState reinforced_release{
        {8.0, -1.0, -3.0},
        {700.022436616558, 21.000673098497, -21.000673098497},
    };

    const auto forward_state = [&](Milliseconds elapsed){
        if (elapsed <= 150ms){
            return interpolate(neutral, wind_up, (double)elapsed.count() / 150.0);
        }
        if (elapsed <= 390ms){
            return interpolate(wind_up, release, (double)(elapsed - 150ms).count() / 240.0);
        }
        if (elapsed <= 540ms){
            return interpolate(release, reinforced_release, (double)(elapsed - 390ms).count() / 150.0);
        }
        return interpolate(reinforced_release, neutral, (double)(elapsed - 540ms).count() / 180.0);
    };

    controller.issue_gyro_motion(&scope, 1440ms, [=](Milliseconds elapsed){
        if (elapsed <= 720ms){
            return forward_state(elapsed);
        }

        // Right-edge sampling means 735ms mirrors the 720ms forward sample,
        // while 1440ms mirrors the 15ms sample. Acceleration stays neutral
        // and inverse angular velocity returns the orientation to its start.
        const GyroState forward = forward_state(1440ms - elapsed + 15ms);
        return GyroState{{}, {
            -forward.angular_velocity_dps.x,
            -forward.angular_velocity_dps.y,
            -forward.angular_velocity_dps.z,
        }};
    });
    controller.wait_for_all(&scope);
}


}
}
