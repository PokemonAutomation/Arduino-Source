/*  Day Skippers
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwSh_Commands_DaySkippers.h"
#include "PokemonSwSh_Messages_DaySkippers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void skipper_init_view(SwitchControllerContext& context){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_init_view()
    );
}
void skipper_auto_recovery(SwitchControllerContext& context){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_auto_recovery()
    );
}
void skipper_rollback_year_full(SwitchControllerContext& context, bool date_us){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_rollback_year_full(date_us)
    );
}
void skipper_rollback_year_sync(SwitchControllerContext& context){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_rollback_year_sync()
    );
}
void skipper_increment_day(SwitchControllerContext& context, bool date_us){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_day(date_us)
    );
}
void skipper_increment_month(SwitchControllerContext& context, uint8_t days){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_month(days)
    );
}
void skipper_increment_all(SwitchControllerContext& context){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_all()
    );
}
void skipper_increment_all_rollback(SwitchControllerContext& context){
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_all_rollback()
    );
}



int register_message_converters_pokemon_skippers(){
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_INIT_VIEW,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_init_view() - ";
            if (body.size() != sizeof(pabb_skipper_init_view)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_init_view*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_AUTO_RECOVERY,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_auto_recovery() - ";
            if (body.size() != sizeof(pabb_skipper_auto_recovery)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_auto_recovery*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_FULL,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_rollback_year_full() - ";
            if (body.size() != sizeof(pabb_skipper_rollback_year_full)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_rollback_year_full*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", date_us = " << params->date_us;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_SYNC,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_rollback_year_sync() - ";
            if (body.size() != sizeof(pabb_skipper_rollback_year_sync)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_rollback_year_sync*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_INCREMENT_DAY,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_increment_day() - ";
            if (body.size() != sizeof(pabb_skipper_increment_day)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_increment_day*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", date_us = " << params->date_us;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_INCREMENT_MONTH,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_increment_month() - ";
            if (body.size() != sizeof(pabb_skipper_increment_month)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_increment_month*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", days = " << (unsigned)params->days;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_increment_all() - ";
            if (body.size() != sizeof(pabb_skipper_increment_all)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_increment_all*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL_ROLLBACK,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "skipper_increment_all_rollback() - ";
            if (body.size() != sizeof(pabb_skipper_increment_all_rollback)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_skipper_increment_all_rollback*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShDaySkippers = register_message_converters_pokemon_skippers();



}
}

