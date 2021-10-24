/*  Pokemon Automation Push Button Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PushButtons_H
#define PokemonAutomation_NintendoSwitch_PushButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void pbf_wait                   (const BotBaseContext& context, uint16_t ticks);
void pbf_press_button           (const BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_press_dpad             (const BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_left_joystick     (const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_right_joystick    (const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_mash_button            (const BotBaseContext& context, Button button, uint16_t ticks);

void start_program_flash        (const BotBaseContext& context, uint16_t ticks);
void grip_menu_connect_go_home  (const BotBaseContext& context);
void end_program_loop           (const BotBaseContext& context);


class DeviceRequest_pbf_wait : public BotBaseRequest{
public:
    pabb_pbf_wait params;
    DeviceRequest_pbf_wait(uint16_t ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_PBF_WAIT, params);
    }
};
class DeviceRequest_pbf_press_button : public BotBaseRequest{
public:
    pabb_pbf_press_button params;
    DeviceRequest_pbf_press_button(Button button, uint16_t hold_ticks, uint16_t release_ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button = button;
        params.hold_ticks = hold_ticks;
        params.release_ticks = release_ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_PBF_PRESS_BUTTON, params);
    }
};
class DeviceRequest_pbf_press_dpad : public BotBaseRequest{
public:
    pabb_pbf_press_dpad params;
    DeviceRequest_pbf_press_dpad(DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.dpad = position;
        params.hold_ticks = hold_ticks;
        params.release_ticks = release_ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_PBF_PRESS_DPAD, params);
    }
};
class DeviceRequest_pbf_move_left_joystick : public BotBaseRequest{
public:
    pabb_pbf_move_joystick params;
    DeviceRequest_pbf_move_left_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.x = x;
        params.y = y;
        params.hold_ticks = hold_ticks;
        params.release_ticks = release_ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L, params);
    }
};
class DeviceRequest_pbf_move_right_joystick : public BotBaseRequest{
public:
    pabb_pbf_move_joystick params;
    DeviceRequest_pbf_move_right_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.x = x;
        params.y = y;
        params.hold_ticks = hold_ticks;
        params.release_ticks = release_ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R, params);
    }
};
class DeviceRequest_pbf_mash_button : public BotBaseRequest{
public:
    pabb_pbf_mash_button params;
    DeviceRequest_pbf_mash_button(Button button, uint16_t ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button = button;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_MASH_BUTTON, params);
    }
};




}
}
#endif
