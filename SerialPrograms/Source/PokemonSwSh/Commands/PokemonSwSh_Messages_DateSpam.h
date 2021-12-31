/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Messages_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Messages_DateSpam_H

#include "Common/PokemonSwSh/PokemonSwSh_Protocol_DateSpam.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "PokemonSwSh_Commands_DateSpam.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_home_to_date_time : public BotBaseRequest{
public:
    pabb_home_to_date_time params;
    DeviceRequest_home_to_date_time(bool to_date_change, bool fast)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.to_date_change = to_date_change;
        params.fast = fast;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_HOME_TO_DATE_TIME, params);
    }
};
class DeviceRequest_neutral_date_skip : public BotBaseRequest{
public:
    pabb_neutral_date_skip params;
    DeviceRequest_neutral_date_skip()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_NEUTRAL_DATE_SKIP, params);
    }
};
class DeviceRequest_roll_date_forward_1 : public BotBaseRequest{
public:
    pabb_roll_date_forward_1 params;
    DeviceRequest_roll_date_forward_1(bool fast)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.fast = fast;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1, params);
    }
};
class DeviceRequest_roll_date_backward_N : public BotBaseRequest{
public:
    pabb_roll_date_backward_N params;
    DeviceRequest_roll_date_backward_N(uint8_t skips, bool fast)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.skips = skips;
        params.fast = fast;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_ROLL_DATE_BACKWARD_N, params);
    }
};
class DeviceRequest_home_roll_date_enter_game : public BotBaseRequest{
public:
    pabb_home_roll_date_enter_game params;
    DeviceRequest_home_roll_date_enter_game(bool rollback_year)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.rollback_year = rollback_year;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_HOME_ROLL_DATE_ENTER_GAME, params);
    }
};
class DeviceRequest_touch_date_from_home : public BotBaseRequest{
public:
    pabb_touch_date_from_home params;
    DeviceRequest_touch_date_from_home(uint16_t settings_to_home_delay)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.settings_to_home_delay = settings_to_home_delay;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_TOUCH_DATE_FROM_HOME, params);
    }
};
class DeviceRequest_rollback_hours_from_home : public BotBaseRequest{
public:
    pabb_rollback_hours_from_home params;
    DeviceRequest_rollback_hours_from_home(uint8_t hours, uint16_t settings_to_home_delay)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.hours = hours;
        params.settings_to_home_delay = settings_to_home_delay;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_ROLLBACK_HOURS_FROM_HOME, params);
    }
};




}
}
#endif
