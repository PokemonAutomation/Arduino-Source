/*  Nintendo Switch Gyro Motion
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include "Common/Cpp/CancellableScope.h"
#include "NintendoSwitch_Gyro.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

namespace{

bool is_finite(const GyroVector& vector){
    return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z);
}

GyroVector transform_vector(ControllerClass controller_class, const GyroVector& vector){
    switch (controller_class){
    case ControllerClass::NintendoSwitch_LeftJoycon:
    case ControllerClass::NintendoSwitch_ProController:
        return vector;
    case ControllerClass::NintendoSwitch_RightJoycon:
        // This is a 180-degree rotation about X, so the basis remains right-handed.
        return {vector.x, -vector.y, -vector.z};
    default:
        throw std::invalid_argument("Gyro motion is unsupported for this controller class.");
    }
}

}


bool is_finite(const GyroState& state){
    return is_finite(state.acceleration_g) && is_finite(state.angular_velocity_dps);
}
GyroState transform_gyro_state(ControllerClass controller_class, const GyroState& state){
    return {
        transform_vector(controller_class, state.acceleration_g),
        transform_vector(controller_class, state.angular_velocity_dps),
    };
}
GyroVector interpolate(const GyroVector& from, const GyroVector& to, double progress){
    return {
        from.x + (to.x - from.x) * progress,
        from.y + (to.y - from.y) * progress,
        from.z + (to.z - from.z) * progress,
    };
}
GyroState interpolate(const GyroState& from, const GyroState& to, double progress){
    return {
        interpolate(from.acceleration_g, to.acceleration_g, progress),
        interpolate(from.angular_velocity_dps, to.angular_velocity_dps, progress),
    };
}


GyroMotion prepare_gyro_motion(
    Cancellable* cancellable,
    Milliseconds duration,
    const GyroFunction& function
){
    if (duration <= Milliseconds::zero()){
        return {};
    }
    if (!function){
        throw std::invalid_argument("Gyro motion function must not be empty.");
    }

    GyroMotion motion;
    std::vector<GyroMotionSample>& samples = motion.m_samples;
    const auto duration_count = duration.count();
    const auto interval_count = GYRO_TRAJECTORY_SAMPLE_INTERVAL.count();
    const auto sample_count = duration_count / interval_count + (duration_count % interval_count != 0);
    if (static_cast<uintmax_t>(sample_count) > samples.max_size()){
        throw std::length_error("Gyro motion duration requires too many samples.");
    }
    samples.reserve(static_cast<size_t>(sample_count));
    Milliseconds elapsed = Milliseconds::zero();
    while (elapsed < duration){
        if (cancellable){
            cancellable->throw_if_cancelled();
        }

        const Milliseconds chunk = std::min(GYRO_TRAJECTORY_SAMPLE_INTERVAL, duration - elapsed);
        elapsed += chunk;
        const GyroState canonical = function(elapsed);
        if (!is_finite(canonical)){
            throw std::invalid_argument("Gyro motion values must be finite.");
        }
        samples.emplace_back(GyroMotionSample{
            chunk,
            canonical,
        });
    }
    return motion;
}

Milliseconds GyroMotion::duration() const{
    Milliseconds total = Milliseconds::zero();
    for (const GyroMotionSample& sample : m_samples){
        total += sample.duration;
    }
    return total;
}

void GyroMotion::append_inverse_rotation(Cancellable* cancellable){
    if (empty()){
        return;
    }
    if (duration() > Milliseconds::max() / 2 || m_samples.size() > m_samples.max_size() / 2){
        throw std::length_error("Gyro motion with inverse rotation is too long.");
    }

    // Build separately so cancellation cannot leave a partially appended inverse.
    std::vector<GyroMotionSample> combined;
    combined.reserve(m_samples.size() * 2);
    for (const GyroMotionSample& sample : m_samples){
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        combined.push_back(sample);
    }
    for (auto iter = m_samples.rbegin(); iter != m_samples.rend(); ++iter){
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        const GyroVector& omega = iter->state.angular_velocity_dps;
        combined.push_back({iter->duration, {{}, {-omega.x, -omega.y, -omega.z}}});
    }
    m_samples.swap(combined);
}


namespace Internal{

int32_t gyro_transport_value(double value, double scale, const char* label){
    if (!std::isfinite(value)){
        throw std::invalid_argument(std::string(label) + " must be finite.");
    }
    const double scaled = value * scale;
    if (scaled < static_cast<double>(std::numeric_limits<int32_t>::min())
        || scaled > static_cast<double>(std::numeric_limits<int32_t>::max())){
        throw std::invalid_argument(std::string(label) + " is outside the firmware transport range.");
    }
    return static_cast<int32_t>(std::round(scaled));
}

namespace{

void validate_transport_state(const GyroState& state){
    gyro_transport_value(state.acceleration_g.x, GYRO_ACCEL_TRANSPORT_SCALE, "Gyro acceleration X");
    gyro_transport_value(state.acceleration_g.y, GYRO_ACCEL_TRANSPORT_SCALE, "Gyro acceleration Y");
    gyro_transport_value(state.acceleration_g.z, GYRO_ACCEL_TRANSPORT_SCALE, "Gyro acceleration Z");
    gyro_transport_value(state.angular_velocity_dps.x, GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE, "Gyro angular velocity X");
    gyro_transport_value(state.angular_velocity_dps.y, GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE, "Gyro angular velocity Y");
    gyro_transport_value(state.angular_velocity_dps.z, GYRO_ANGULAR_VELOCITY_TRANSPORT_SCALE, "Gyro angular velocity Z");
}

}

std::vector<GyroMotionSample> transform_gyro_motion(
    Cancellable* cancellable,
    ControllerClass controller_class,
    const GyroMotion& motion
){
    if (motion.empty()){
        return {};
    }
    transform_gyro_state(controller_class, {});
    std::vector<GyroMotionSample> samples;
    samples.reserve(motion.samples().size());
    for (const GyroMotionSample& sample : motion.samples()){
        if (cancellable){
            cancellable->throw_if_cancelled();
        }
        const GyroState transformed = transform_gyro_state(controller_class, sample.state);
        validate_transport_state(transformed);
        samples.push_back({sample.duration, transformed});
    }
    return samples;
}


}
}
}
