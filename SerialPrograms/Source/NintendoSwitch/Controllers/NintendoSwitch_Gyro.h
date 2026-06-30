/*  Nintendo Switch Gyro Motion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Gyro_H
#define PokemonAutomation_NintendoSwitch_Gyro_H

#include <cstdint>
#include <functional>
#include <vector>
#include "Common/Cpp/Time.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{

class Cancellable;

namespace NintendoSwitch{


struct GyroVector{
    double x = 0;
    double y = 0;
    double z = 0;
};

// Canonical controller frame:
//   +X points toward the trigger/shoulder-button edge.
//   +Y points toward the controller's left when looking at its controls.
//   +Z points out through the controls.
// Positive angular velocity follows the right-hand rule. Acceleration is
// sensor acceleration in g; gravity is not synthesized by this API. This is
// a fixed logical input convention, not a world-space reference frame.
struct GyroState{
    GyroVector acceleration_g;
    GyroVector angular_velocity_dps;
};

using GyroFunction = std::function<GyroState(Milliseconds elapsed)>;

constexpr Milliseconds GYRO_REPORT_INTERVAL{15};

bool is_finite(const GyroState& state);
GyroState transform_gyro_state(ControllerClass controller_class, const GyroState& state);
GyroVector interpolate(const GyroVector& from, const GyroVector& to, double progress);
GyroState interpolate(const GyroState& from, const GyroState& to, double progress);


namespace Internal{

constexpr double GYRO_ACCEL_TRANSPORT_SCALE = 1000000.0;
constexpr double GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE = 1000.0;

// Converts a program-facing physical value to the signed transport unit used
// by the firmware protocol. Sampling calls this for every component so a
// complete trajectory is validated before any scheduler state is changed.
int32_t gyro_transport_value(double value, double scale, const char* label);

// Internal representation used while a trajectory is being validated before
// it reaches the scheduler. It intentionally contains no raw sensor counts.
struct GyroMotionSample{
    Milliseconds duration;
    GyroState state;
};

std::vector<GyroMotionSample> sample_gyro_motion(
    Cancellable* cancellable,
    ControllerClass controller_class,
    Milliseconds duration,
    const GyroFunction& function
);

}


}
}
#endif
