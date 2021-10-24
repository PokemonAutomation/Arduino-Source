/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "PokemonSwSh_Commands_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop(const BotBaseContext& context){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_eggfetcher_loop()
    );
}
void move_while_mashing_B(const BotBaseContext& context, uint16_t duration){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_move_while_mashing_B(duration)
    );
}
void spin_and_mash_A(const BotBaseContext& context, uint16_t duration){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_spin_and_mash_A(duration)
    );
}
void travel_to_spin_location(const BotBaseContext& context){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_travel_to_spin_location()
    );
}
void travel_back_to_lady(const BotBaseContext& context){
    context->issue_request(
        &context.cancelled_bool(),
        DeviceRequest_travel_back_to_lady()
    );
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
}

