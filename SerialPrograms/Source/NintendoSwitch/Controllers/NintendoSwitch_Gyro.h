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

// Host trajectory resolution, independent of the firmware's IMU/report clocks.
constexpr Milliseconds GYRO_TRAJECTORY_SAMPLE_INTERVAL{15};

bool is_finite(const GyroState& state);
GyroState transform_gyro_state(ControllerClass controller_class, const GyroState& state);
GyroVector interpolate(const GyroVector& from, const GyroVector& to, double progress);
GyroState interpolate(const GyroState& from, const GyroState& to, double progress);


// A canonical physical state held for the given duration. These boundaries
// remain intact when a prepared motion is replayed or reversed.
struct GyroMotionSample{
    Milliseconds duration;
    GyroState state;
};

class GyroMotion{
public:
    bool empty() const{ return m_samples.empty(); }
    Milliseconds duration() const;
    const std::vector<GyroMotionSample>& samples() const{ return m_samples; }

    // Append the inverse rotation of the entire current motion: reverse the
    // stored samples, negate angular velocity, and use zero acceleration.
    // Durations are preserved, including a short final sample. This undoes
    // commanded rotation; it does not reset orientation or undo translation.
    // Cancellation or failure leaves the original motion unchanged.
    void append_inverse_rotation(Cancellable* cancellable = nullptr);

private:
    friend GyroMotion prepare_gyro_motion(Cancellable*, Milliseconds, const GyroFunction&);
    std::vector<GyroMotionSample> m_samples;
};

// Evaluate the callback once at each interval's right edge, including the
// final endpoint, and retain only the resulting finite canonical states.
// Non-positive duration is a no-op. Controller frame conversion and transport
// range validation happen before scheduling, so this motion can be reused on
// different controllers without resampling or converting its axes twice.
GyroMotion prepare_gyro_motion(
    Cancellable* cancellable,
    Milliseconds duration,
    const GyroFunction& function
);


namespace Internal{

constexpr double GYRO_ACCEL_TRANSPORT_SCALE = 1000000.0;
constexpr double GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE = 1000.0;

// Converts a program-facing physical value to the signed transport unit used
// by the firmware protocol. Frame conversion checks every component so a
// complete trajectory is validated before any scheduler state is changed.
int32_t gyro_transport_value(double value, double scale, const char* label);

// Convert and validate a copy; the prepared canonical motion is unchanged.
std::vector<GyroMotionSample> transform_gyro_motion(
    Cancellable* cancellable,
    ControllerClass controller_class,
    const GyroMotion& motion
);

}


}
}
#endif
