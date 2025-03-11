/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Messages_EggRoutines_H
#define PokemonAutomation_PokemonSwSh_Messages_EggRoutines_H

#include "Common/PokemonSwSh/PokemonSwSh_Protocol_EggRoutines.h"
#include "ClientSource/Connection/BotBaseMessage.h"
#include "PokemonSwSh_Commands_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_eggfetcher_loop : public BotBaseRequest{
public:
    pabb_eggfetcher_loop params;
    DeviceRequest_eggfetcher_loop()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_EGG_FETCHER_LOOP, params);
    }
};
class DeviceRequest_move_while_mashing_B : public BotBaseRequest{
public:
    pabb_move_while_mashing_B params;
    DeviceRequest_move_while_mashing_B(uint16_t duration)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.duration = duration;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_MOVE_WHILE_MASHING_B, params);
    }
};
class DeviceRequest_spin_and_mash_A : public BotBaseRequest{
public:
    pabb_spin_and_mash_A params;
    DeviceRequest_spin_and_mash_A(uint16_t duration)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.duration = duration;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SPIN_AND_MASH_A, params);
    }
};
class DeviceRequest_travel_to_spin_location : public BotBaseRequest{
public:
    pabb_travel_to_spin_location params;
    DeviceRequest_travel_to_spin_location()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_TRAVEL_TO_SPIN_LOCATION, params);
    }
};
class DeviceRequest_travel_back_to_lady : public BotBaseRequest{
public:
    pabb_travel_back_to_lady params;
    DeviceRequest_travel_back_to_lady()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_TRAVEL_BACK_TO_LADY, params);
    }
};




}
}
#endif
