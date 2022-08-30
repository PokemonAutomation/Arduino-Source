/*  Push Button Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  The functions to send Switch commands to the micro controller.
 *  The timing of the commnads are measured in ticks. One second is 125 ticks.
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Commands_PushButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
//    class BotBaseContext;
namespace NintendoSwitch{

//  Block the current thread until all the Switch commands sent to the micro controller are executed.
void pbf_wait                   (BotBaseContext& context, uint16_t ticks);
//  Press a Switch controller button (excluding D-Pad). Hold the button for `hold_ticks`, then release it for `release_ticks`.
//  The buttons are defined in Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h. Examples include BUTTON_A, BUTTON_ZL.
//  The buttons also include clicking joysticks: BUTTON_LCLICK, BUTTON_RCLICK.
//  D-Pad buttons and directional movements of joysticks are controlled by separate functions.
void pbf_press_button           (BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks);
//  Press a Switch controller D-Pad button. Hold the button for `hold_ticks`, then release it for `release_ticks`.
//  The buttons are defined in Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h. Examples include DPAD_DOWN, DPAD_UP_RIGHT.
void pbf_press_dpad             (BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);

//  Move left joystick towards a 2D direction. Hold the direction for `hold_ticks`, then release it for `release_ticks`.
//  The direction is specified by (x, y):
//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down
//  Example: move the joystick fully left: (x, y) = (0, 128)
//           move the joystick upper-right: (x, y) = (255, 0
void pbf_move_left_joystick     (BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
//  Move right joystick towards a 2D direction. Hold the direction for `hold_ticks`, then release it for `release_ticks`.
//  The direction is specified by (x, y):
//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down
//  Example: move the joystick fully left: (x, y) = (0, 128)
//           move the joystick upper-right: (x, y) = (255, 0
void pbf_move_right_joystick    (BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
//  Mash a Switch controller button (excluding D-Pad) repeatedly for `ticks` ticks.
//  The buttons are defined in Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h. Examples include BUTTON_A, BUTTON_ZL.
//  The buttons also include clicking joysticks: BUTTON_LCLICK, BUTTON_RCLICK.
void pbf_mash_button            (BotBaseContext& context, Button button, uint16_t ticks);

void start_program_flash        (BotBaseContext& context, uint16_t ticks);
void grip_menu_connect_go_home  (BotBaseContext& context);

// void pbf_controller_state(
//     BotBaseContext& context,
//     Button button,
//     DpadPosition position,
//     uint8_t left_x, uint8_t left_y,
//     uint8_t right_x, uint8_t right_y,
//     uint8_t ticks
// );

void pbf_controller_state(
    BotBaseContext& context,
    Button button,
    DpadPosition position,
    uint8_t left_x, uint8_t left_y,
    uint8_t right_x, uint8_t right_y,
    uint16_t ticks
);



}
}
#endif
