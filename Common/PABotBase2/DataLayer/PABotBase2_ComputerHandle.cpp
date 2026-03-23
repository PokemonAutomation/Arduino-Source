/*  Computer Connection
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#include <string.h>
#include "SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "PABotBase2_MessageProtocol.h"
#include "PABotBase2_ComputerHandle.h"

#include "ComputerConnection.h" //  REMOVE

namespace PokemonAutomation{
namespace PABotBase2{



void ComputerHandle::send_message_u32(uint8_t id, uint32_t data){
    pabb2_Message_Response_u32 response;
    response.message_bytes = sizeof(pabb2_Message_Response_u32);
    response.opcode = PABB2_MESSAGE_OPCODE_RET_U32;
    response.id = id;
    response.data = data;
    data_connection.reliable_send(&response, response.message_bytes);

//    data_connection.send_oob_info_u32(response.message_bytes);
}
void ComputerHandle::send_message_data(uint8_t id, uint16_t bytes, const void* data){
    pabb2_Message_Response_Data response;
    response.message_bytes = sizeof(pabb2_Message_Response_Data) + bytes;
    response.opcode = PABB2_MESSAGE_OPCODE_RET_DATA;
    response.id = id;
    data_connection.reliable_send(&response, sizeof(pabb2_Message_Response_Data));
    data_connection.reliable_send(data, bytes);
}
void ComputerHandle::process_completed_message(){
    const pabb2_MessageHeader* header = (const pabb2_MessageHeader*)m_buffer;

    switch (header->opcode){
    case PABB2_MESSAGE_OPCODE_PROTOCOL_VERSION:
        send_message_u32(header->id, PABB2_MESSAGE_PROTOCOL_VERSION);
        return;
    case PABB2_MESSAGE_OPCODE_FIRMWARE_VERSION:
        send_message_u32(header->id, PABB2_FIRMWARE_VERSION);
        return;
    case PABB2_MESSAGE_OPCODE_DEVICE_IDENTIFIER:
        send_message_u32(header->id, PABB2_DEVICE_ID);
        return;
    case PABB2_MESSAGE_OPCODE_DEVICE_NAME:
        send_message_data(header->id, sizeof(PABB2_DEVICE_NAME) - 1, PABB2_DEVICE_NAME);
        return;
    case PABB2_MESSAGE_OPCODE_CONTROLLER_LIST:
        send_message_data(header->id, sizeof(PABB_CONTROLLER_LIST), PABB_CONTROLLER_LIST);
        return;
    case PABB2_MESSAGE_OPCODE_CQ_CAPACITY:
        send_message_u32(header->id, PABB2_CommandQueue_SLOTS);
        return;

    }


}



bool ComputerHandle::run_events(){
    bool ret = m_connection.run_events();

    //  Check for reset.
    if (m_connection.reset_flag_set()){
        m_index = 0;
        m_connection.clear_reset_flag();
        return true;
    }

    //  Try to read a message.

    //  Make sure we have enough for the length.
    if (m_index < sizeof(uint16_t)){
        m_index += m_connection.reliable_recv(m_buffer, sizeof(pabb2_MessageHeader) - m_index);
        if (m_index < sizeof(uint16_t)){
            return ret;
        }
    }

    uint16_t message_bytes;
    memcpy(&message_bytes, m_buffer, sizeof(uint16_t));

    //  Message is not complete.
    if (m_index < message_bytes){
        size_t read = m_connection.reliable_recv(m_buffer, sizeof(pabb2_MessageHeader) - m_index);
        m_index += read;
        if (m_index < message_bytes){
            return ret;
        }
    }

//    data_connection.send_oob_info_u32(123);

    process_completed_message();
    m_index = 0;


    return true;
}



}
}
