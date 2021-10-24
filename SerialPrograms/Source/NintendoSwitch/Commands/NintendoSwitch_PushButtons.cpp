/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "NintendoSwitch_Device.h"
#include "NintendoSwitch_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void pbf_wait(const BotBaseContext& context, uint16_t ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_pbf_wait(ticks)
    );
}
void pbf_press_button(const BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_pbf_press_button(button, hold_ticks, release_ticks)
    );
}
void pbf_press_dpad(const BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_pbf_press_dpad(position, hold_ticks, release_ticks)
    );
}
void pbf_move_left_joystick(const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_pbf_move_left_joystick(x, y, hold_ticks, release_ticks)
    );
}
void pbf_move_right_joystick(const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_pbf_move_right_joystick(x, y, hold_ticks, release_ticks)
    );
}
void pbf_mash_button(const BotBaseContext& context, Button button, uint16_t ticks){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_pbf_mash_button(button, ticks)
    );
}

void start_program_flash(const BotBaseContext& context, uint16_t ticks){
    for (uint16_t c = 0; c < ticks; c += 50){
        set_leds(context, true);
        pbf_wait(context, 25);
        set_leds(context, false);
        pbf_wait(context, 25);
    }
}
void grip_menu_connect_go_home(const BotBaseContext& context){
    pbf_press_button(context, BUTTON_L | BUTTON_R, 10, 40);
    pbf_press_button(context, BUTTON_A, 10, 140);
    pbf_press_button(context, BUTTON_HOME, 10, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
}
void end_program_loop(const BotBaseContext& context){
#if 0
    pbf_wait(device, 15 * TICKS_PER_SECOND);
    while (true){
        pbf_press_button(device, BUTTON_ZL, 10, 15 * TICKS_PER_SECOND - 10);
    }
#endif
}



}
}


