/*  Push Button Framework (pbf_*)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The functions to send Switch commands to the micro controller.
 *  The timing of the commands are measured in milliseconds.
 *
 *  You should use these functions (instead of ssf_*) whenever possible since
 *  these are cleaner and easier to use/understand.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Commands_PushButtons_H

#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/Joycon/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


using namespace std::chrono_literals;


//  Wait for this duration on the Switch.
void pbf_wait           (ProControllerContext& context, Milliseconds duration);

//  Press a Switch controller button (excluding D-Pad). Hold the button for `hold`, then release it for `release`.
//  The buttons are defined in NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h. Examples include BUTTON_A, BUTTON_ZL.
//  The buttons also include clicking joysticks: BUTTON_LCLICK, BUTTON_RCLICK.
//  D-Pad buttons and directional movements of joysticks are controlled by separate functions.
void pbf_press_button       (ProControllerContext& context, Button button, Milliseconds hold, Milliseconds release);

//  Press a Switch controller D-Pad button. Hold the button for `hold`, then release it for `release`.
//  The buttons are defined in NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h. Examples include DPAD_DOWN, DPAD_UP_RIGHT.
void pbf_press_dpad     (ProControllerContext& context, DpadPosition position, Milliseconds hold, Milliseconds release);

//  Move left joystick towards a 2D direction. Hold the direction for `hold`, then release it for `release`.
//  The direction is specified by a JoystickPosition (x, y) with floating point values:
//  x = -1 : left
//  x = 0 : neutral
//  x = +1 : right
//  y = -1 : up
//  y = 0 : neutral
//  y = +1 : down
//  Values outside of [-1, +1] are scaled back into the unit circle before being sent.
//  Example: move the joystick fully left: (x, y) = (-1, 0)
//           move the joystick upper-right: (x, y) = (+1, -1)
void pbf_move_left_joystick     (ProControllerContext& context, const JoystickPosition& position, Milliseconds hold, Milliseconds release);

//  Move right joystick towards a 2D direction. Hold the direction for `hold`, then release it for `release`.
//  The direction is specified by a JoystickPosition (x, y) with floating point values:
//  x = -1 : left
//  x = 0 : neutral
//  x = +1 : right
//  y = -1 : up
//  y = 0 : neutral
//  y = +1 : down
//  Values outside of [-1, +1] are scaled back into the unit circle before being sent.
//  Example: move the joystick fully left: (x, y) = (-1, 0)
//           move the joystick upper-right: (x, y) = (+1, -1)
void pbf_move_right_joystick    (ProControllerContext& context, const JoystickPosition& position, Milliseconds hold, Milliseconds release);

//  Mash a Switch controller button (excluding D-Pad) repeatedly for `duration`.
//  The buttons are defined in NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h. Examples include BUTTON_A, BUTTON_ZL.
//  The buttons also include clicking joysticks: BUTTON_LCLICK, BUTTON_RCLICK.
void pbf_mash_button            (ProControllerContext& context, Button button, Milliseconds duration);

void grip_menu_connect_go_home  (ProControllerContext& context);



//
//  Press all the following buttons/joysticks simultaneously for the specified
//  duration. No wait is added at the end. Thus you can issue these back-to-back
//  to simulate buttons being pressed and released concurrently with other
//  buttons being held down the whole time.
//
//  Note that this function does not play well with unfinished ssf_* functions.
//  If there is a pending ssf_* function, a conflicting button press by this
//  function (and possibly more) will be delayed - thus causing the buttons in
//  this function call to not issue simultaneously. The exact behavior is
//  undefined so you should never do this.
//
//  The sole purpose of this function is for keyboard commands. For in-program
//  button overlapping, you should use ssf_* directly. (though lots of existing
//  programs already use this for overlapping)
//
void pbf_controller_state(
    ProControllerContext& context,
    Button button,
    DpadPosition position,
    const JoystickPosition& left_joystick,
    const JoystickPosition& right_joystick,
    Milliseconds duration
);




void pbf_wait           (JoyconContext& context, Milliseconds duration);
void pbf_press_button   (JoyconContext& context, Button button, Milliseconds hold, Milliseconds release);
void pbf_move_joystick  (JoyconContext& context, const JoystickPosition& position, Milliseconds hold, Milliseconds release);
void pbf_mash_button    (JoyconContext& context, Button button, Milliseconds duration);

void pbf_controller_state(
    JoyconContext& context,
    Button button,
    const JoystickPosition& joystick,
    Milliseconds duration
);


}
}
#endif
