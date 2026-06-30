/*  Nintendo Switch Gyro Quaternion Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 */

#include <cmath>
#include <numbers>
#include <stdexcept>
#include "NintendoSwitch_GyroQuaternion.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

namespace{

bool is_finite(const GyroQuaternion& quaternion){
    return std::isfinite(quaternion.x) && std::isfinite(quaternion.y)
        && std::isfinite(quaternion.z) && std::isfinite(quaternion.w);
}
GyroQuaternion multiply(const GyroQuaternion& left, const GyroQuaternion& right){
    return {
        left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y,
        left.w * right.y + left.y * right.w + left.z * right.x - left.x * right.z,
        left.w * right.z + left.z * right.w + left.x * right.y - left.y * right.x,
        left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z,
    };
}
GyroQuaternion normalize(const GyroQuaternion& quaternion){
    const double norm = std::sqrt(
        quaternion.x * quaternion.x + quaternion.y * quaternion.y
        + quaternion.z * quaternion.z + quaternion.w * quaternion.w
    );
    if (!std::isfinite(norm) || norm == 0){
        throw std::invalid_argument("Gyro quaternion is not normalizable.");
    }
    GyroQuaternion normalized{
        quaternion.x / norm, quaternion.y / norm,
        quaternion.z / norm, quaternion.w / norm,
    };
    if (!is_finite(normalized)){
        throw std::invalid_argument("Gyro quaternion integration produced a non-finite value.");
    }
    return normalized;
}

}


GyroQuaternion integrate_angular_velocity(
    const GyroQuaternion& current,
    const GyroVector& angular_velocity_dps,
    Milliseconds duration
){
    if (!is_finite(current) || !std::isfinite(angular_velocity_dps.x)
        || !std::isfinite(angular_velocity_dps.y) || !std::isfinite(angular_velocity_dps.z)
        || duration < Milliseconds::zero()){
        throw std::invalid_argument("Gyro quaternion integration requires finite values and a non-negative duration.");
    }

    const double seconds = static_cast<double>(duration.count()) / 1000.0;
    const double scale = std::numbers::pi / 180.0 * seconds;
    const double x = angular_velocity_dps.x * scale;
    const double y = angular_velocity_dps.y * scale;
    const double z = angular_velocity_dps.z * scale;
    const double angle = std::sqrt(x * x + y * y + z * z);
    const double vector_scale = angle == 0 ? 0.5 : std::sin(angle / 2.0) / angle;
    const GyroQuaternion delta{x * vector_scale, y * vector_scale, z * vector_scale, std::cos(angle / 2.0)};
    return normalize(multiply(current, delta));
}

void pack_quaternion(
    OemController_State0x30_GyroQuaternion& output,
    const GyroQuaternion& orientation,
    uint64_t timestamp_ms
){
    if (!is_finite(orientation)){
        throw std::invalid_argument("Cannot pack a non-finite gyro quaternion.");
    }

    // Packing assumes unit components. Normalize here so callers cannot reach
    // the narrowing conversion below with a finite but out-of-range value.
    const GyroQuaternion unit = normalize(orientation);
    const double values[] = {unit.x, unit.y, unit.z, unit.w};
    int max_index = 0;
    for (int i = 1; i < 4; ++i){
        if (std::fabs(values[i]) > std::fabs(values[max_index])){
            max_index = i;
        }
    }

    output.set_packing_mode(2);
    output.set_max_index(max_index);
    const double sign = values[max_index] < 0 ? -1.0 : 1.0;
    int32_t components[3];
    for (int i = 0; i < 3; ++i){
        components[i] = static_cast<int32_t>(values[(max_index + i + 1) & 3] * 0x40000000 * sign);
    }
    output.set_last_sample_0(components[0] >> 10);
    output.set_last_sample_1(components[1] >> 10);
    output.set_last_sample_2(components[2] >> 10);
    output.set_timestamp_start(timestamp_ms & 0x7ff);
    output.set_timestamp_count(3);
}


}
}
