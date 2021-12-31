/*  Day Skippers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Messages_DaySkippers_H
#define PokemonAutomation_PokemonSwSh_Messages_DaySkippers_H

#include "Common/PokemonSwSh/PokemonSwSh_Protocol_DaySkippers.h"
#include "ClientSource/Connection/BotBaseMessage.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class DeviceRequest_skipper_init_view : public BotBaseRequest{
public:
    pabb_skipper_init_view params;
    DeviceRequest_skipper_init_view()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_INIT_VIEW, params);
    }
};
class DeviceRequest_skipper_auto_recovery : public BotBaseRequest{
public:
    pabb_skipper_auto_recovery params;
    DeviceRequest_skipper_auto_recovery()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_AUTO_RECOVERY, params);
    }
};
class DeviceRequest_skipper_rollback_year_full : public BotBaseRequest{
public:
    pabb_skipper_rollback_year_full params;
    DeviceRequest_skipper_rollback_year_full(bool date_us)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.date_us = date_us;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_FULL, params);
    }
};
class DeviceRequest_skipper_rollback_year_sync : public BotBaseRequest{
public:
    pabb_skipper_rollback_year_sync params;
    DeviceRequest_skipper_rollback_year_sync()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_SYNC, params);
    }
};
class DeviceRequest_skipper_increment_day : public BotBaseRequest{
public:
    pabb_skipper_increment_day params;
    DeviceRequest_skipper_increment_day(bool date_us)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.date_us = date_us;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_INCREMENT_DAY, params);
    }
};
class DeviceRequest_skipper_increment_month : public BotBaseRequest{
public:
    pabb_skipper_increment_month params;
    DeviceRequest_skipper_increment_month(uint8_t days)
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
        params.days = days;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_INCREMENT_MONTH, params);
    }
};
class DeviceRequest_skipper_increment_all : public BotBaseRequest{
public:
    pabb_skipper_increment_all params;
    DeviceRequest_skipper_increment_all()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL, params);
    }
};
class DeviceRequest_skipper_increment_all_rollback : public BotBaseRequest{
public:
    pabb_skipper_increment_all_rollback params;
    DeviceRequest_skipper_increment_all_rollback()
        : BotBaseRequest(true)
    {
        params.seqnum = 0;
    }
    virtual BotBaseMessage message() const override{
        return BotBaseMessage(PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL_ROLLBACK, params);
    }
};



}
}
#endif
