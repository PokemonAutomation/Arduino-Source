/*  Nintendo Switch Controller Buttons
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


std::string button_to_string(Button button){
    std::string str;
    if (button & BUTTON_Y)          str += "Y ";
    if (button & BUTTON_B)          str += "B ";
    if (button & BUTTON_A)          str += "A ";
    if (button & BUTTON_X)          str += "X ";
    if (button & BUTTON_L)          str += "L ";
    if (button & BUTTON_R)          str += "R ";
    if (button & BUTTON_ZL)         str += "ZL ";
    if (button & BUTTON_ZR)         str += "ZR ";
    if (button & BUTTON_MINUS)      str += "- ";
    if (button & BUTTON_PLUS)       str += "+ ";
    if (button & BUTTON_LCLICK)     str += "LJ ";
    if (button & BUTTON_RCLICK)     str += "RJ ";
    if (button & BUTTON_HOME)       str += "HOME ";
    if (button & BUTTON_CAPTURE)    str += "CAPTURE ";
    if (button & BUTTON_GR)         str += "GR ";
    if (button & BUTTON_GL)         str += "GL ";
    if (button & BUTTON_UP)         str += "Up ";
    if (button & BUTTON_RIGHT)      str += "Right ";
    if (button & BUTTON_DOWN)       str += "Down ";
    if (button & BUTTON_LEFT)       str += "Left ";
    if (button & BUTTON_LEFT_SL)    str += "Left-SL ";
    if (button & BUTTON_LEFT_SR)    str += "Left-SR ";
    if (button & BUTTON_RIGHT_SL)   str += "Right-SL ";
    if (button & BUTTON_RIGHT_SR)   str += "Right-SR ";
    if (button & BUTTON_C)          str += "C ";
    if (str.empty()){
        str = "none";
    }
    if (str.back() == ' '){
        str.pop_back();
    }
    return str;
}
std::string dpad_to_string(DpadPosition dpad){
    switch (dpad){
    case DPAD_UP            : return "up";
    case DPAD_UP_RIGHT      : return "up-right";
    case DPAD_RIGHT         : return "right";
    case DPAD_DOWN_RIGHT    : return "down-right";
    case DPAD_DOWN          : return "down";
    case DPAD_DOWN_LEFT     : return "down-left";
    case DPAD_LEFT          : return "left";
    case DPAD_UP_LEFT       : return "up-left";
    case DPAD_NONE          : return "none";
    }
    return "unknown";
}



}
}
