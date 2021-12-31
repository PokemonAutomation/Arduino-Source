/*  Push Button Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Messages_PushButtons_H
#define PokemonAutomation_NintendoSwitch_Messages_PushButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


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


class DeviceRequest_controller_state : public BotBaseRequest{
public:
    pabb_controller_state params;
    DeviceRequest_controller_state(
        Button button,
        DpadPosition dpad,
        uint8_t left_joystick_x,
        uint8_t left_joystick_y,
        uint8_t right_joystick_x,
        uint8_t right_joystick_y,
        uint8_t ticks
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button = button;
        params.dpad = dpad;
        params.left_joystick_x = left_joystick_x;
        params.left_joystick_y = left_joystick_y;
        params.right_joystick_x = right_joystick_x;
        params.right_joystick_y = right_joystick_y;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_CONTROLLER_STATE, params);
    }
};



}
}
#endif
