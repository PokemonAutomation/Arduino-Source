/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "ClientSource/Connection/BotBase.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwShEggRoutines.h"


#if 0
void eggfetcher_loop(void){
    eggfetcher_loop(*PokemonAutomation::global_connection);
}
void move_while_mashing_B(uint16_t duration){
    move_while_mashing_B(*PokemonAutomation::global_connection, duration);
}
void spin_and_mash_A(uint16_t duration){
    spin_and_mash_A(*PokemonAutomation::global_connection, duration);
}
void travel_to_spin_location(void){
    travel_to_spin_location(*PokemonAutomation::global_connection);
}
void travel_back_to_lady(void){
    travel_back_to_lady(*PokemonAutomation::global_connection);
}
#endif



namespace PokemonAutomation{



void eggfetcher_loop(const BotBaseContext& context){
    pabb_eggfetcher_loop params;
    context->issue_request<PABB_MSG_COMMAND_EGG_FETCHER_LOOP>(&context.cancelled_bool(), params);
}
void move_while_mashing_B(const BotBaseContext& context, uint16_t duration){
    pabb_move_while_mashing_B params;
    params.duration = duration;
    context->issue_request<PABB_MSG_COMMAND_MOVE_WHILE_MASHING_B>(&context.cancelled_bool(), params);
}
void spin_and_mash_A(const BotBaseContext& context, uint16_t duration){
    pabb_spin_and_mash_A params;
    params.duration = duration;
    context->issue_request<PABB_MSG_COMMAND_SPIN_AND_MASH_A>(&context.cancelled_bool(), params);
}
void travel_to_spin_location(const BotBaseContext& context){
    pabb_travel_to_spin_location params;
    context->issue_request<PABB_MSG_COMMAND_TRAVEL_TO_SPIN_LOCATION>(&context.cancelled_bool(), params);
}
void travel_back_to_lady(const BotBaseContext& context){
    pabb_travel_back_to_lady params;
    context->issue_request<PABB_MSG_COMMAND_TRAVEL_BACK_TO_LADY>(&context.cancelled_bool(), params);
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


}

