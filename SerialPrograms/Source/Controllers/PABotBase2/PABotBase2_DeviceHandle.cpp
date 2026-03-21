/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "PABotBase2_DeviceHandle.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{



void DeviceHandle::connect(){
    m_device_protocol = query_u32(PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION);
    m_device_firmware_version = query_u32(PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION);
    m_device_id = query_u32(PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER);
    m_command_queue_size = (uint8_t)query_u32(PABB2_MESSAGE_OPCODE_CQ_CAPACITY);

}

void DeviceHandle::on_recv(const void* data, size_t bytes){




}


void DeviceHandle::send_data(const void* data, size_t bytes){
    //  Must be called under the lock.

    const char* ptr = (const char*)data;
    while (bytes > 0){
        if (m_stopping){
            throw OperationCancelledException();
        }
        size_t sent;
        try{
            sent = m_connection.send(ptr, bytes);
        }catch (...){
            m_stopping = true;
            throw;
        }
        ptr += sent;
        bytes -= sent;
    }
}
void DeviceHandle::send_request(pabb2_MessageHeader_Request& request){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        if (m_stopping){
            throw OperationCancelledException();
        }

        request.id = m_seqnum;

        //  Wait until the slot is available.
        auto iter = m_pending_requests.find(request.id);
        if (iter != m_pending_requests.end()){
            m_cv.wait(lg);
            continue;
        }

        m_pending_requests[request.id];
        break;
    }
}
std::string DeviceHandle::wait_for_response(uint8_t id){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        if (m_stopping){
            throw OperationCancelledException();
        }

        auto iter = m_pending_requests.find(id);
        if (iter == m_pending_requests.end()){
            throw InternalProgramError(
                &m_logger,
                PA_CURRENT_FUNCTION,
                "Attempted to wait for an ID that doesn't exist."
            );
        }

        if (iter->second.response.empty()){
            m_cv.wait(lg);
            continue;
        }

        std::string str = std::move(iter->second.response);
        m_pending_requests.erase(iter);
        return str;
    }
}


uint32_t DeviceHandle::query_u32(uint8_t opcode){
    pabb2_MessageHeader_Request request;
    request.message_bytes = sizeof(pabb2_MessageHeader_Request);
    request.opcode = opcode;

    send_request(request);

    std::string response = wait_for_response(request.id);
    if (response.size() != sizeof(pabb2_Message_Response_u32)){
        throw InternalProgramError(
            &m_logger,
            PA_CURRENT_FUNCTION,
            "Expected reponse length of: " + std::to_string(sizeof(pabb2_Message_Response_u32))
        );
    }

    const pabb2_Message_Response_u32* msg = (const pabb2_Message_Response_u32*)response.c_str();
    return msg->data;
}



}
}
