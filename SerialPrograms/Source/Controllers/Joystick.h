/*  Joystick
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_Joystick_H
#define PokemonAutomation_Controllers_Joystick_H

namespace PokemonAutomation{


struct JoystickPosition{
    double x = 0.0;
    double y = 0.0;

    JoystickPosition() = default;
    JoystickPosition(double x, double y)
        : x(x), y(y)
    {}

    bool is_neutral() const{
        return x == 0 && y == 0;
    }

    friend bool operator==(const JoystickPosition& lhs, const JoystickPosition& rhs){
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    friend bool operator!=(const JoystickPosition& lhs, const JoystickPosition& rhs){
        return lhs.x != rhs.x || lhs.y != rhs.y;
    }
};


struct JoystickCardinal{
    double magnitude = 0.0;
    double angle = 0.0;

    JoystickCardinal(double magnitude, double angle)
        : magnitude(magnitude), angle(angle)
    {}

    operator JoystickPosition() const;
};



}
#endif
