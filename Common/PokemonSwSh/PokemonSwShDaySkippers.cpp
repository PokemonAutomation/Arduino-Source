/*  Day Skippers
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwShDaySkippers.h"

using namespace PokemonAutomation;

void skipper_init_view(void){
    skipper_init_view(*global_connection);
}
void skipper_init_view(BotBase& device){
    pabb_skipper_init_view params;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_INIT_VIEW>(params);
}
void skipper_auto_recovery(void){
    skipper_auto_recovery(*global_connection);
}
void skipper_auto_recovery(BotBase& device){
    pabb_skipper_auto_recovery params;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_AUTO_RECOVERY>(params);
}
void skipper_rollback_year_full(bool date_us){
    skipper_rollback_year_full(*global_connection, date_us);
}
void skipper_rollback_year_full(BotBase& device, bool date_us){
    pabb_skipper_rollback_year_full params;
    params.date_us = date_us;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_FULL>(params);
}
void skipper_rollback_year_sync(void){
    skipper_rollback_year_sync(*global_connection);
}
void skipper_rollback_year_sync(BotBase& device){
    pabb_skipper_rollback_year_sync params;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_SYNC>(params);
}
void skipper_increment_day(bool date_us){
    skipper_increment_day(*global_connection, date_us);
}
void skipper_increment_day(BotBase& device, bool date_us){
    pabb_skipper_increment_day params;
    params.date_us = date_us;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_INCREMENT_DAY>(params);
}
void skipper_increment_month(uint8_t days){
    skipper_increment_month(*global_connection, days);
}
void skipper_increment_month(BotBase& device, uint8_t days){
    pabb_skipper_increment_month params;
    params.days = days;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_INCREMENT_MONTH>(params);
}
void skipper_increment_all(void){
    skipper_increment_all(*global_connection);
}
void skipper_increment_all(BotBase& device){
    pabb_skipper_increment_all params;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL>(params);
}
void skipper_increment_all_rollback(void){
    skipper_increment_all_rollback(*global_connection);
}
void skipper_increment_all_rollback(BotBase& device){
    pabb_skipper_increment_all_rollback params;
    device.issue_request<PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL_ROLLBACK>(params);
}



int register_message_converters_pokemon_skippers(){
    register_message_converter(
        PABB_MSG_COMMAND_SKIPPER_INIT_VIEW,
        [](const std::string& body){
            std::stringstream ss;
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
            std::stringstream ss;
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
            std::stringstream ss;
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
            std::stringstream ss;
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
            std::stringstream ss;
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
            std::stringstream ss;
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
            std::stringstream ss;
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
            std::stringstream ss;
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
