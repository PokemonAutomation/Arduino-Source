/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/MessageProtocol.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Libraries/Compiler.h"
#include "ClientSource/Connection/BotBase.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{
    BotBase* global_connection = nullptr;
}
using namespace PokemonAutomation;

void initialize_framework(uint8_t program_id){}

void end_program_callback(){
    end_program_callback(*global_connection);
}
void end_program_callback(BotBase& device){
    pabb_end_program_callback params;
    device.issue_request<PABB_MSG_COMMAND_END_PROGRAM_CALLBACK>(params);
}

void set_leds(bool on){
    set_leds(*global_connection, on);
}
void set_leds(BotBase& device, bool on){
    pabb_MsgCommandSetLeds params;
    params.on = on;
    device.issue_request<PABB_MSG_COMMAND_SET_LED_STATE>(params);
}

uint32_t system_clock(void){
    return system_clock(*global_connection);
}
uint32_t system_clock(BotBase& device){
    pabb_system_clock params;
    pabb_MsgAckRequestI32 response;
    device.issue_request_and_wait<PABB_MSG_REQUEST_CLOCK, PABB_MSG_ACK_REQUEST_I32>(params, response);
    return response.data;
}

void pbf_wait(uint16_t ticks){
    pbf_wait(*global_connection, ticks);
}
void pbf_wait(BotBase& device, uint16_t ticks){
    pabb_pbf_wait params;
    params.ticks = ticks;
    device.issue_request<PABB_MSG_COMMAND_PBF_WAIT>(params);
}

void pbf_press_button(Button button, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_press_button(*global_connection, button, hold_ticks, release_ticks);
}
void pbf_press_button(BotBase& device, Button button, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_press_button params;
    params.button = button;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    device.issue_request<PABB_MSG_COMMAND_PBF_PRESS_BUTTON>(params);
}

void pbf_press_dpad(DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_press_dpad(*global_connection, position, hold_ticks, release_ticks);
}
void pbf_press_dpad(BotBase& device, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_press_dpad params;
    params.dpad = position;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    device.issue_request<PABB_MSG_COMMAND_PBF_PRESS_DPAD>(params);
}

void pbf_move_left_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_move_left_joystick(*global_connection, x, y, hold_ticks, release_ticks);
}
void pbf_move_left_joystick(BotBase& device, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_move_joystick params;
    params.x = x;
    params.y = y;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    device.issue_request<PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L>(params);
}
void pbf_move_right_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pbf_move_right_joystick(*global_connection, x, y, hold_ticks, release_ticks);
}
void pbf_move_right_joystick(BotBase& device, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks){
    pabb_pbf_move_joystick params;
    params.x = x;
    params.y = y;
    params.hold_ticks = hold_ticks;
    params.release_ticks = release_ticks;
    device.issue_request<PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R>(params);
}

void pbf_mash_button(Button button, uint16_t ticks){
    pbf_mash_button(*global_connection, button, ticks);
}
void pbf_mash_button(BotBase& device, Button button, uint16_t ticks){
    pabb_pbf_mash_button params;
    params.button = button;
    params.ticks = ticks;
    device.issue_request<PABB_MSG_COMMAND_MASH_BUTTON>(params);
}

void start_program_flash(uint16_t ticks){
    start_program_flash(*global_connection, ticks);
}
void start_program_flash(BotBase& device, uint16_t ticks){
    for (uint16_t c = 0; c < ticks; c += 50){
        set_leds(device, true);
        pbf_wait(device, 25);
        set_leds(device, false);
        pbf_wait(device, 25);
    }
}

void grip_menu_connect_go_home(void){
    grip_menu_connect_go_home(*global_connection);
}
void grip_menu_connect_go_home(BotBase& device){
    pbf_press_button(device, BUTTON_L | BUTTON_R, 10, 40);
    pbf_press_button(device, BUTTON_A, 10, 140);
    pbf_press_button(device, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY);
}

void end_program_loop(void){
    end_program_loop(*global_connection);
}
void end_program_loop(BotBase& device){
#if 0
    pbf_wait(device, 15 * TICKS_PER_SECOND);
    while (true){
        pbf_press_button(device, BUTTON_ZL, 10, 15 * TICKS_PER_SECOND - 10);
    }
#endif
}



