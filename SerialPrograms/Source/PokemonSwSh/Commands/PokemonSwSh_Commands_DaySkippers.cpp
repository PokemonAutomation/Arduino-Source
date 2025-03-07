/*  Day Skippers
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwSh_Commands_DaySkippers.h"
#include "PokemonSwSh_Messages_DaySkippers.h"

#ifdef PA_OFFICIAL
//#define PA_CC_DAY_SKIPPERS
#endif

#ifdef PA_CC_DAY_SKIPPERS
#include "../../Internal/SerialPrograms/PokemonSwSh_DateSkippers.h"
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{


void skipper_init_view(ProControllerContext& context){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::init_view(context);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_init_view()
    );
#endif
}
void skipper_auto_recovery(ProControllerContext& context){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::auto_recovery(context);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_auto_recovery()
    );
#endif
}
void skipper_rollback_year_full(ProControllerContext& context, bool date_us){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::rollback_year_full(context, date_us);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_rollback_year_full(date_us)
    );
#endif
}
void skipper_rollback_year_sync(ProControllerContext& context){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::rollback_year_sync(context);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_rollback_year_sync()
    );
#endif
}
void skipper_increment_day(ProControllerContext& context, bool date_us){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::increment_day(context, date_us);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_day(date_us)
    );
#endif
}
void skipper_increment_month(ProControllerContext& context, uint8_t days){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::increment_month(context, days);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_month(days)
    );
#endif
}
void skipper_increment_all(ProControllerContext& context){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::increment_all(context);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_all()
    );
#endif
}
void skipper_increment_all_rollback(ProControllerContext& context){
#ifdef PA_CC_DAY_SKIPPERS
    DaySkippers::increment_all_rollback(context);
#else
    context->send_botbase_request(
        &context,
        DeviceRequest_skipper_increment_all_rollback()
    );
#endif
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

