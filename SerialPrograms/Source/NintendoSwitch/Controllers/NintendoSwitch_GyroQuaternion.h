/*  Nintendo Switch Gyro Quaternion Helpers
 *
 *  From: https://github.com/PokemonAutomation/
 */

#ifndef PokemonAutomation_NintendoSwitch_GyroQuaternion_H
#define PokemonAutomation_NintendoSwitch_GyroQuaternion_H

#include <cstdint>
#include "Common/ControllerStates/NintendoSwitch_OemController_State.h"
#include "NintendoSwitch_Gyro.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct GyroQuaternion{
    double x = 0;
    double y = 0;
    double z = 0;
    double w = 1;
};

GyroQuaternion integrate_angular_velocity(
    const GyroQuaternion& current,
    const GyroVector& angular_velocity_dps,
    Milliseconds duration
);

void pack_quaternion(
    OemController_State0x30_GyroQuaternion& output,
    const GyroQuaternion& orientation,
    uint64_t timestamp_ms
);


}
}
#endif
