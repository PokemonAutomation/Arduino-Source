/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/Compiler.h"
#include "Common/MessageProtocol.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Libraries/Compiler.h"
#include "ClientSource/Connection/BotBase.h"

//#include <iostream>
//using std::cout;
//using std::endl;


#if 0
void end_program_callback(){
    end_program_callback(*PokemonAutomation::global_connection);
}
void initialize_framework(uint8_t program_id){}
void set_leds(bool on){
    set_leds(*PokemonAutomation::global_connection, on);
}
uint32_t system_clock(void){
    return system_clock(*PokemonAutomation::global_connection);
}
void pbf_wait(uint16_t ticks){
    pbf_wait(*PokemonAutomation::global_connection, ticks);
}
void pbf_press_button(Button button, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_press_button(*PokemonAutomation::global_connection, button, hold_ticks, release_ticks);
}
void pbf_press_dpad(DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_press_dpad(*PokemonAutomation::global_connection, position, hold_ticks, release_ticks);
}
void pbf_move_left_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_move_left_joystick(*PokemonAutomation::global_connection, x, y, hold_ticks, release_ticks);
}
void pbf_move_right_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_move_right_joystick(*PokemonAutomation::global_connection, x, y, hold_ticks, release_ticks);
}
void pbf_mash_button(Button button, uint16_t ticks){
    pbf_mash_button(*PokemonAutomation::global_connection, button, ticks);
}
void start_program_flash(uint16_t ticks){
    start_program_flash(*PokemonAutomation::global_connection, ticks);
}
void grip_menu_connect_go_home(void){
    grip_menu_connect_go_home(*PokemonAutomation::global_connection);
}
void end_program_loop(void){
    end_program_loop(*PokemonAutomation::global_connection);
}
#endif






namespace PokemonAutomation{
    BotBaseContext* global_connection = nullptr;



void end_program_callback(const BotBaseContext& context){
    pabb_end_program_callback params;
    context->issue_request<PABB_MSG_COMMAND_END_PROGRAM_CALLBACK>(&context.cancelled_bool(), params);
}

void set_leds(const BotBaseContext& context, bool on){
    pabb_MsgCommandSetLeds params;
    params.on = on;
    context->issue_request<PABB_MSG_COMMAND_SET_LED_STATE>(&context.cancelled_bool(), params);
}

uint32_t system_clock(const BotBaseContext& context){
    pabb_system_clock params;
    pabb_MsgAckRequestI32 response;
    context->issue_request_and_wait<PABB_MSG_REQUEST_CLOCK, PABB_MSG_ACK_REQUEST_I32>(&context.cancelled_bool(), params, response);
    return response.data;
}
void pbf_wait(const BotBaseContext& context, uint16_t ticks){
    pabb_pbf_wait params;
    params.ticks = ticks;
    context->issue_request<PABB_MSG_COMMAND_PBF_WAIT>(&context.cancelled_bool(), params);
}
void pbf_press_button(const BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_press_button params;
    params.button = button;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    context->issue_request<PABB_MSG_COMMAND_PBF_PRESS_BUTTON>(&context.cancelled_bool(), params);
}
void pbf_press_dpad(const BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_press_dpad params;
    params.dpad = position;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    context->issue_request<PABB_MSG_COMMAND_PBF_PRESS_DPAD>(&context.cancelled_bool(), params);
}
void pbf_move_left_joystick(const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_move_joystick params;
    params.x = x;
    params.y = y;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    context->issue_request<PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L>(&context.cancelled_bool(), params);
}
void pbf_move_right_joystick(const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_move_joystick params;
    params.x = x;
    params.y = y;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    context->issue_request<PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R>(&context.cancelled_bool(), params);
}
void pbf_mash_button(const BotBaseContext& context, Button button, uint16_t ticks){
    pabb_pbf_mash_button params;
    params.button = button;
    params.ticks = ticks;
    context->issue_request<PABB_MSG_COMMAND_MASH_BUTTON>(&context.cancelled_bool(), params);
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
    pbf_press_button(context, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY);
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


