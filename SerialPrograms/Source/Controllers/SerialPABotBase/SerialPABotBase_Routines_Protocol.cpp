/*  SerialPABotBase Routines (Protocol)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "SerialPABotBase_Routines_Protocol.h"

namespace PokemonAutomation{
namespace SerialPABotBase{


uint32_t protocol_version(BotBaseController& botbase){
    pabb_MsgAckRequestI32 response;
    botbase.issue_request_and_wait(
        DeviceRequest_protocol_version()
    ).convert<PABB_MSG_ACK_REQUEST_I32>(botbase.logger(), response);
    return response.data;
}
uint32_t program_version(BotBaseController& botbase){
    pabb_MsgAckRequestI32 response;
    botbase.issue_request_and_wait(
        DeviceRequest_program_version()
    ).convert<PABB_MSG_ACK_REQUEST_I32>(botbase.logger(), response);
    return response.data;
}
uint8_t program_id(BotBaseController& botbase){
    pabb_MsgAckRequestI8 response;
    botbase.issue_request_and_wait(
        DeviceRequest_program_id()
    ).convert<PABB_MSG_ACK_REQUEST_I8>(botbase.logger(), response);
    return response.data;
}
std::string program_name(BotBaseController& botbase){
    BotBaseMessage response = botbase.issue_request_and_wait(
        DeviceRequest_program_name(),
        nullptr
    );
    return response.body.substr(sizeof(seqnum_t));
}
std::vector<pabb_ControllerID> controller_list(BotBaseController& botbase){
    BotBaseMessage response = botbase.issue_request_and_wait(
        DeviceRequest_controller_list(),
        nullptr
    );
    size_t bytes = response.body.size() - sizeof(seqnum_t);
    if (bytes % sizeof(uint32_t) != 0){
        throw SerialProtocolException(
            botbase.logger(), PA_CURRENT_FUNCTION,
            "Device sent back invalid reponse length: " + std::to_string(response.body.size())
        );
    }
    std::vector<pabb_ControllerID> ret(bytes / sizeof(uint32_t));
    memcpy(ret.data(), response.body.data() + sizeof(seqnum_t), bytes);
    return ret;
}
uint8_t device_queue_size(BotBaseController& botbase){
    pabb_MsgAckRequestI8 response;
    botbase.issue_request_and_wait(
        DeviceRequest_queue_size()
    ).convert<PABB_MSG_ACK_REQUEST_I8>(botbase.logger(), response);
    return response.data;

}
uint32_t read_controller_mode(BotBaseController& botbase){
    pabb_MsgAckRequestI32 response;
    botbase.issue_request_and_wait(
        DeviceRequest_read_controller_mode()
    ).convert<PABB_MSG_ACK_REQUEST_I32>(botbase.logger(), response);
    return response.data;
}
uint32_t change_controller_mode(BotBaseController& botbase, uint32_t mode){
    pabb_MsgAckRequestI32 response;
    botbase.issue_request_and_wait(
        DeviceRequest_change_controller_mode(mode)
    ).convert<PABB_MSG_ACK_REQUEST_I32>(botbase.logger(), response);
    return response.data;
}


}
}
