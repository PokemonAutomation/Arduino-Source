/*  Scalar Button Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Messages_ScalarButtons_H
#define PokemonAutomation_NintendoSwitch_Messages_ScalarButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_Protocol_ScalarButtons.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



class DeviceRequest_ssf_flush_pipeline : public BotBaseRequest{
public:
    pabb_ssf_flush_pipeline params;
    DeviceRequest_ssf_flush_pipeline()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_FLUSH_PIPELINE, params);
    }
};
class DeviceRequest_ssf_do_nothing : public BotBaseRequest{
public:
    pabb_ssf_do_nothing params;
    DeviceRequest_ssf_do_nothing(uint16_t ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_DO_NOTHING, params);
    }
};
class DeviceRequest_ssf_press_button : public BotBaseRequest{
public:
    pabb_ssf_press_button params;
    DeviceRequest_ssf_press_button(
        Button button,
        uint16_t delay,
        uint16_t hold,
        uint8_t cool
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button = button;
        params.delay = delay;
        params.hold = hold;
        params.cool = cool;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_PRESS_BUTTON, params);
    }
};
class DeviceRequest_ssf_press_dpad : public BotBaseRequest{
public:
    pabb_ssf_press_dpad params;
    DeviceRequest_ssf_press_dpad(
        DpadPosition position,
        uint16_t delay,
        uint16_t hold,
        uint8_t cool
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.position = position;
        params.delay = delay;
        params.hold = hold;
        params.cool = cool;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_PRESS_DPAD, params);
    }
};
class DeviceRequest_ssf_press_joystick : public BotBaseRequest{
public:
    pabb_ssf_press_joystick params;
    bool left;
    DeviceRequest_ssf_press_joystick(
        bool p_left,
        uint8_t x, uint8_t y,
        uint16_t delay,
        uint16_t hold,
        uint8_t cool
    )
        : BotBaseRequest(true)
        , left(p_left)
    {
        params.seqnum = 0;
        params.x = x;
        params.y = y;
        params.delay = delay;
        params.hold = hold;
        params.cool = cool;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(
            left ? PABB_MSG_COMMAND_SSF_PRESS_JOYSTICK_L : PABB_MSG_COMMAND_SSF_PRESS_JOYSTICK_R,
            params
        );
    }
};
class DeviceRequest_ssf_mash1_button : public BotBaseRequest{
public:
    pabb_ssf_mash1_button params;
    DeviceRequest_ssf_mash1_button(Button button, uint16_t ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button = button;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_MASH1_BUTTON, params);
    }
};
class DeviceRequest_ssf_mash2_button : public BotBaseRequest{
public:
    pabb_ssf_mash2_button params;
    DeviceRequest_ssf_mash2_button(Button button0, Button button1, uint16_t ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.button0 = button0;
        params.button1 = button1;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_MASH2_BUTTON, params);
    }
};
class DeviceRequest_ssf_mash_AZs : public BotBaseRequest{
public:
    pabb_ssf_mash_AZs params;
    DeviceRequest_ssf_mash_AZs(uint16_t ticks)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.ticks = ticks;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_MASH_AZS, params);
    }
};
class DeviceRequest_ssf_issue_scroll : public BotBaseRequest{
public:
    pabb_ssf_issue_scroll params;
    DeviceRequest_ssf_issue_scroll(
        ssf_ScrollDirection direction,
        uint16_t delay,
        uint16_t hold,
        uint8_t cool
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.direction = direction;
        params.delay = delay;
        params.hold = hold;
        params.cool = cool;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SSF_SCROLL, params);
    }
};



}
}
#endif
