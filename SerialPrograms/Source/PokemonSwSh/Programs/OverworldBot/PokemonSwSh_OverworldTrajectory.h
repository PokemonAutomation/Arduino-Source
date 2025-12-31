/*  Overworld Trajectory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldTrajectory_H
#define PokemonAutomation_PokemonSwSh_OverworldTrajectory_H

#include <stdint.h>
#include "Common/Cpp/Time.h"
#include "Controllers/JoystickTools.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

struct Trajectory{
    Milliseconds distance_in_millis;
    double joystick_fx;
    double joystick_fy;

    Trajectory() = default;

    //  Old constructor for TRAJECTORY_TABLE.
    Trajectory(
        uint16_t distance_in_ticks,
        uint8_t joystick_x,
        uint8_t joystick_y
    )
        : distance_in_millis(distance_in_ticks * Milliseconds(8))
        , joystick_fx(JoystickTools::linear_u8_to_float(joystick_x))
        , joystick_fy(-JoystickTools::linear_u8_to_float(joystick_y))
    {}

    Trajectory(
        void*,
        Milliseconds distance_in_millis,
        double joystick_x,
        double joystick_y
    )
        : distance_in_millis(distance_in_millis)
        , joystick_fx(joystick_x)
        , joystick_fy(joystick_y)
    {}
};

Trajectory get_trajectory_float(double delta_x, double delta_y);


}
}
}
#endif
