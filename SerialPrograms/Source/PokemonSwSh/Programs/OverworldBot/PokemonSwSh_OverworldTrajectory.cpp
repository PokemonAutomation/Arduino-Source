/*  Overworld Trajectory
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "PokemonSwSh_OverworldTrajectory.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



const Trajectory TRAJECTORY_TABLE[8][6] = {
    {{400,128,  0},{420,160,  0},{420,190,  0},{480,208,  0},{480,208,  0},{480,208,  0},},
    {{210,128,  0},{220,164,  0},{250,200,  0},{280,208,  0},{300,209,  0},{320,209,  0},},
    {{120,128,  0},{120,192,  0},{140,209,  0},{180,224,  0},{220,224,  0},{260,232,  0},},
    {{ 60,128,  0},{ 65,224,  0},{ 95,255, 32},{125,255, 48},{160,255, 48},{200,255, 48},},
    {{  0,128,128},{ 40,255,110},{ 70,255,105},{100,255,100},{130,255, 95},{160,255, 90},},
    {{ 35,128,255},{ 45,255,255},{ 70,255,170},{ 90,255,160},{115,255,140},{140,255,128},},
    {{ 55,128,255},{ 60,192,255},{ 75,255,255},{ 90,255,192},{110,255,176},{135,255,144},},
    {{ 75,128,255},{ 80,160,255},{ 85,216,255},{ 95,255,224},{115,255,192},{130,255,176},},
};
Trajectory get_trajectory_int(int delta_x, int delta_y){
    delta_x = std::max(delta_x, -5);
    delta_x = std::min(delta_x, +5);
    delta_y = std::max(delta_y, -4);
    delta_y = std::min(delta_y, +3);

    if (delta_x < 0){
        Trajectory entry = get_trajectory_int(-delta_x, delta_y);
        entry.joystick_x = (uint8_t)(256 - entry.joystick_x);
        return entry;
    }

    return TRAJECTORY_TABLE[delta_y + 4][delta_x];
}
Trajectory get_trajectory_float(double delta_x, double delta_y){
    delta_x *= 10;
    delta_y *= 10;

    int int_x = (int)std::floor(delta_x);
    int int_y = (int)std::floor(delta_y);

    double frac_x = delta_x - int_x;
    double frac_y = delta_y - int_y;

    Trajectory corner11 = get_trajectory_int(int_x, int_y);
    Trajectory corner01 = get_trajectory_int(int_x + 1, int_y);
    Trajectory corner10 = get_trajectory_int(int_x, int_y + 1);
    Trajectory corner00 = get_trajectory_int(int_x + 1, int_y + 1);

    double top_x = corner11.joystick_x * (1.0 - frac_x) + corner01.joystick_x * frac_x;
    double bot_x = corner10.joystick_x * (1.0 - frac_x) + corner00.joystick_x * frac_x;
    double x = top_x * (1.0 - frac_y) + bot_x * frac_y;
//    cout << "top_x = " << top_x << endl;
//    cout << "bot_x = " << bot_x << endl;
//    cout << "x = " << x << endl;

    double top_y = corner11.joystick_y * (1.0 - frac_y) + corner10.joystick_y * frac_y;
    double bot_y = corner01.joystick_y * (1.0 - frac_y) + corner00.joystick_y * frac_y;
    double y = top_y * (1.0 - frac_x) + bot_y * frac_x;

    double top_d = corner11.distance_in_ticks * (1.0 - frac_x) + corner01.distance_in_ticks * frac_x;
    double bot_d = corner10.distance_in_ticks * (1.0 - frac_x) + corner00.distance_in_ticks * frac_x;
    double d = top_d * (1.0 - frac_y) + bot_d * frac_y;

    x -= 128;
    y -= 128;
    double scale = std::max(std::abs(x), std::abs(y));
    if (scale == 0){
        x = 128;
        y = 128;
    }else{
        scale = 128 / scale;
        x *= scale;
        y *= scale;
    }
    x += 128;
    y += 128;

    d = std::max(d, 0.0);
    d = std::min(d, 65535.);
    x = std::max(x, 0.0);
    y = std::max(y, 0.0);
    x = std::min(x, 255.);
    y = std::min(y, 255.);

    return Trajectory{(uint16_t)d, (uint8_t)x, (uint8_t)y};
}


}
}
}
