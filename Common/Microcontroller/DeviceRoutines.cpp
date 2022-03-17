/*  Device Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "DeviceRoutines.h"

namespace PokemonAutomation{
namespace Microcontroller{


uint32_t protocol_version(const BotBaseContext& context){
    pabb_MsgAckRequestI32 response;
    context.issue_request_and_wait(
        DeviceRequest_protocol_version()
    ).convert<PABB_MSG_ACK_REQUEST_I32>(context.botbase().logger(), response);
    return response.data;
}
uint32_t program_version(const BotBaseContext& context){
    pabb_MsgAckRequestI32 response;
    context.issue_request_and_wait(
        DeviceRequest_program_version()
    ).convert<PABB_MSG_ACK_REQUEST_I32>(context.botbase().logger(), response);
    return response.data;
}
uint8_t program_id(const BotBaseContext& context){
    pabb_MsgAckRequestI8 response;
    context.issue_request_and_wait(
        DeviceRequest_program_id()
    ).convert<PABB_MSG_ACK_REQUEST_I8>(context.botbase().logger(), response);
    return response.data;
}


}
}
