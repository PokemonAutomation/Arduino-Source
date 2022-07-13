/*  Push Button Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Commands_PushButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
//    class BotBaseContext;
namespace NintendoSwitch{


void pbf_wait                   (BotBaseContext& context, uint16_t ticks);
void pbf_press_button           (BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_press_dpad             (BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);

//  x = 0 : left
//  x = 128 : neutral
//  x = 255 : right
//  y = 0 : up
//  y = 128 : neutral
//  y = 255 : down
void pbf_move_left_joystick     (BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_right_joystick    (BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
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
