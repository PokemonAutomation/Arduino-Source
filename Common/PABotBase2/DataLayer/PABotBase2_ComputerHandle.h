/*  Computer Connection
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Firmware_ComputerHandle_H
#define PokemonAutomation_Firmware_ComputerHandle_H

#include "Common/Cpp/StreamConnections/PollingStreamConnections.h"
#include "PABotbase2_Config.h"

namespace PokemonAutomation{
namespace PABotBase2{


class ComputerHandle{
public:
    ComputerHandle(ReliableStreamConnectionPolling& connection)
        : m_connection(connection)
        , m_index(0)
    {}

    //  Returns true if there may be more work to do.
    bool run_events();
    void wait_for_event(uint16_t milliseconds){
        m_connection.wait_for_event(milliseconds);
    }

private:
    void send_message_u32(uint8_t id, uint32_t data);
    void send_message_data(uint8_t id, uint16_t bytes, const void* data);
    void process_completed_message();

private:
    ReliableStreamConnectionPolling& m_connection;

    uint8_t m_index;
    char m_buffer[PABB2_MAX_INCOMING_MESSAGE_SIZE];
};



}
}
#endif
