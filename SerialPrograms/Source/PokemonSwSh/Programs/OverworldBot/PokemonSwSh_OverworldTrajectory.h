/*  Overworld Trajectory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldTrajectory_H
#define PokemonAutomation_PokemonSwSh_OverworldTrajectory_H

#include <stdint.h>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

struct Trajectory{
    uint16_t distance_in_ticks;
    uint8_t joystick_x;
    uint8_t joystick_y;
};

Trajectory get_trajectory_float(double delta_x, double delta_y);


}
}
}
#endif
