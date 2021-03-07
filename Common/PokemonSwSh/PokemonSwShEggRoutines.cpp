/*  Egg Routines
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwShEggRoutines.h"

using namespace PokemonAutomation;

void eggfetcher_loop(void){
    eggfetcher_loop(*global_connection);
}
void eggfetcher_loop(BotBase& device){
    pabb_eggfetcher_loop params;
    device.issue_request<PABB_MSG_COMMAND_EGG_FETCHER_LOOP>(params);
}
void move_while_mashing_B(uint16_t duration){
    move_while_mashing_B(*global_connection, duration);
}
void move_while_mashing_B(BotBase& device, uint16_t duration){
    pabb_move_while_mashing_B params;
    params.duration = duration;
    device.issue_request<PABB_MSG_COMMAND_MOVE_WHILE_MASHING_B>(params);
}
void spin_and_mash_A(uint16_t duration){
    spin_and_mash_A(*global_connection, duration);
}
void spin_and_mash_A(BotBase& device, uint16_t duration){
    pabb_spin_and_mash_A params;
    params.duration = duration;
    device.issue_request<PABB_MSG_COMMAND_SPIN_AND_MASH_A>(params);
}
void travel_to_spin_location(void){
    travel_to_spin_location(*global_connection);
}
void travel_to_spin_location(BotBase& device){
    pabb_travel_to_spin_location params;
    device.issue_request<PABB_MSG_COMMAND_TRAVEL_TO_SPIN_LOCATION>(params);
}
void travel_back_to_lady(void){
    travel_back_to_lady(*global_connection);
}
void travel_back_to_lady(BotBase& device){
    pabb_travel_back_to_lady params;
    device.issue_request<PABB_MSG_COMMAND_TRAVEL_BACK_TO_LADY>(params);
}



int register_message_converters_eggs(){
    register_message_converter(
        PABB_MSG_COMMAND_EGG_FETCHER_LOOP,
        [](const std::string& body){
            std::stringstream ss;
            ss << "eggfetcher_loop() - ";
            if (body.size() != sizeof(pabb_eggfetcher_loop)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_eggfetcher_loop*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_MOVE_WHILE_MASHING_B,
        [](const std::string& body){
            std::stringstream ss;
            ss << "move_while_mashing_B() - ";
            if (body.size() != sizeof(pabb_move_while_mashing_B)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_move_while_mashing_B*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", duration = " << params->duration;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SPIN_AND_MASH_A,
        [](const std::string& body){
            std::stringstream ss;
            ss << "spin_and_mash_A() - ";
            if (body.size() != sizeof(pabb_spin_and_mash_A)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_spin_and_mash_A*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", duration = " << params->duration;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_TRAVEL_TO_SPIN_LOCATION,
        [](const std::string& body){
            std::stringstream ss;
            ss << "travel_to_spin_location() - ";
            if (body.size() != sizeof(pabb_travel_to_spin_location)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_travel_to_spin_location*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_TRAVEL_BACK_TO_LADY,
        [](const std::string& body){
            std::stringstream ss;
            ss << "travel_back_to_lady() - ";
            if (body.size() != sizeof(pabb_travel_back_to_lady)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_travel_back_to_lady*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShEggRoutines = register_message_converters_eggs();
