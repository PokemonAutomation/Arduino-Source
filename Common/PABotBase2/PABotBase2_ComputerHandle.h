/*  Computer Connection
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Firmware_ComputerHandle_H
#define PokemonAutomation_Firmware_ComputerHandle_H

#include <string.h>
#include "Common/Cpp/StreamConnections/PollingStreamConnections.h"
#include "PABotBase2_MessageProtocol.h"
#include "PABotbase2_Config.h"

namespace PokemonAutomation{
namespace PABotBase2{



class ComputerHandle{
public:
    struct Listener{
        virtual bool execute_message(const MessageHeader* header) = 0;
    };


public:
    ComputerHandle(
        ReliableStreamConnectionPolling& connection,
        Listener* listener = nullptr
    )
        : m_connection(connection)
        , m_listener(listener)
        , m_index(0)
    {}
    void set_listener(Listener* listener){
        m_listener = listener;
    }

    //  Returns true if there may be more work to do.
    bool run_events();
    void wait_for_event(uint16_t milliseconds){
        m_connection.wait_for_event(milliseconds);
    }


public:
    void send_message_invalid_message(uint8_t id);
    void send_message_u32(uint8_t id, const uint32_t& data);
    void send_message_data(
        uint8_t opcode, uint8_t id,
        uint16_t bytes, const void* data
    );


public:
    void send_log_str(const char* str){
        send_message_data(PABB2_MESSAGE_OPCODE_LOG_STRING, 0, strlen(str), str);
    }
    void send_log_label_h32(const char* str, const uint32_t& data){
        send_message_u32_data(PABB2_MESSAGE_OPCODE_LOG_LABEL_H32, 0, data, strlen(str), str);
    }
    void send_log_label_u32(const char* str, const uint32_t& data){
        send_message_u32_data(PABB2_MESSAGE_OPCODE_LOG_LABEL_U32, 0, data, strlen(str), str);
    }
    void send_log_label_i32(const char* str, const uint32_t& data){
        send_message_u32_data(PABB2_MESSAGE_OPCODE_LOG_LABEL_I32, 0, data, strlen(str), str);
    }


private:
    void send_message_u32_data(
        uint8_t opcode, uint8_t id,
        const uint32_t& u32,
        uint16_t bytes, const void* data
    );
    void process_completed_message();


private:
    ReliableStreamConnectionPolling& m_connection;
    Listener* m_listener = nullptr;

    uint8_t m_index;
    char m_buffer[PABB2_MAX_INCOMING_MESSAGE_SIZE];
};



}
}
#endif
