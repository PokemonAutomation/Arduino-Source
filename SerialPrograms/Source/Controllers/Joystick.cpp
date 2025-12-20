/*  Joystick
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Joystick.h"

namespace PokemonAutomation{


JoystickCardinal::operator JoystickPosition() const{
    double radians = angle * (3.1415926535897932385 / 180);
    return JoystickPosition(
        magnitude * std::sin(radians),
        magnitude * std::cos(radians)
    );
}


}
