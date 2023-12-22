/*  Auto Host Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Messages_AutoHosts_H
#define PokemonAutomation_PokemonSwSh_Messages_AutoHosts_H

#include "Common/PokemonSwSh/PokemonSwSh_Protocol_AutoHosts.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "PokemonSwSh_Commands_AutoHosts.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class DeviceRequest_connect_to_internet : public BotBaseRequest{
public:
    pabb_connect_to_internet params;
    DeviceRequest_connect_to_internet(
        uint16_t open_ycomm_delay,
        uint16_t connect_to_internet_delay
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.open_ycomm_delay = open_ycomm_delay;
        params.connect_to_internet_delay = connect_to_internet_delay;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_CONNECT_TO_INTERNET, params);
    }
#if 0
    virtual std::string to_str() const override{
        std::ostringstream ss;
        ss << "connect_to_internet() - ";
        ss << "seqnum = " << (uint64_t)params.seqnum;
        ss << ", open_ycomm_delay = " << params.open_ycomm_delay;
        ss << ", connect_to_internet_delay = " << params.connect_to_internet_delay;
        return ss.str();
    }
#endif
};
class DeviceRequest_home_to_add_friends : public BotBaseRequest{
public:
    pabb_home_to_add_friends params;
    DeviceRequest_home_to_add_friends(
        uint8_t user_slot,
        uint8_t scroll_down,
        bool fix_cursor
    )
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.user_slot = user_slot;
        params.scroll_down = scroll_down;
        params.fix_cursor = fix_cursor;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_HOME_TO_ADD_FRIENDS, params);
    }
};




}
}
}
#endif
