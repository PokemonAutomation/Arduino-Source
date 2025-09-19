/*  Push Button Framework (pbf_*)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  The functions to send Switch commands to the micro controller.
 *  The timing of the commands are measured in ticks. One second is 125 ticks.
 *
 *  **New** Overloads have been added that use milliseconds instead of ticks.
 *  If the underlying controller doesn't support millisecond precision, it will
 *  be rounded up to whatever granularity it does support.
 *
 *  You should use these functions (instead of ssf_*) whenever possible since
 *  these are cleaner and easier to use/understand.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Commands_PushButtons_H

#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


using namespace std::chrono_literals;


//  Wait for this many ticks on the Switch.
void pbf_wait           (ProControllerContext& context, uint16_t ticks);
void pbf_wait           (ProControllerContext& context, Milliseconds duration);

//  Press a Switch controller button (excluding D-Pad). Hold the button for `hold_ticks`, then release it for `release_ticks`.
//  The buttons are defined in Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h. Examples include BUTTON_A, BUTTON_ZL.
//  The buttons also include clicking joysticks: BUTTON_LCLICK, BUTTON_RCLICK.
//  D-Pad buttons and directional movements of joysticks are controlled by separate functions.
void pbf_press_button   (ProControllerContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_press_button   (ProControllerContext& context, Button button, Milliseconds hold, Milliseconds release);

//  Press a Switch controller D-Pad button. Hold the button for `hold_ticks`, then release it for `release_ticks`.
//  The buttons are defined in Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h. Examples include DPAD_DOWN, DPAD_UP_RIGHT.
void pbf_press_dpad     (ProControllerContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_press_dpad     (ProControllerContext& context, DpadPosition position, Milliseconds hold, Milliseconds release);

//  Move left joystick towards a 2D direction. Hold the direction for `hold_ticks`, then release it for `release_ticks`.
//  The direction is specified by (x, y):
//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down
//  Example: move the joystick fully left: (x, y) = (0, 128)
//           move the joystick upper-right: (x, y) = (255, 0)
void pbf_move_left_joystick (ProControllerContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_left_joystick (ProControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release);

//  Move right joystick towards a 2D direction. Hold the direction for `hold_ticks`, then release it for `release_ticks`.
//  The direction is specified by (x, y):
//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down
//  Example: move the joystick fully left: (x, y) = (0, 128)
//           move the joystick upper-right: (x, y) = (255, 0)
void pbf_move_right_joystick    (ProControllerContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_right_joystick    (ProControllerContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release);

//  Mash a Switch controller button (excluding D-Pad) repeatedly for `ticks` ticks.
//  The buttons are defined in Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h. Examples include BUTTON_A, BUTTON_ZL.
//  The buttons also include clicking joysticks: BUTTON_LCLICK, BUTTON_RCLICK.
void pbf_mash_button            (ProControllerContext& context, Button button, uint16_t ticks);
void pbf_mash_button            (ProControllerContext& context, Button button, Milliseconds duration);

//void start_program_flash        (ProControllerContext& context, uint16_t ticks);
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
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint16_t ticks
);
void pbf_controller_state(
    ProControllerContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    Milliseconds duration
);




void pbf_wait           (JoyconContext& context, Milliseconds duration);
void pbf_press_button   (JoyconContext& context, Button button, Milliseconds hold, Milliseconds release);
void pbf_move_joystick  (JoyconContext& context, uint8_t x, uint8_t y, Milliseconds hold, Milliseconds release);
void pbf_mash_button    (JoyconContext& context, Button button, Milliseconds duration);

void pbf_controller_state(
    JoyconContext& context,
    Button button,
    uint8_t x, uint8_t y,
    Milliseconds duration
);


}
}
#endif
