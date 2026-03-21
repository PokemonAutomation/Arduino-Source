/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H
#define PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H

#include "Common/Cpp/StreamConnections/StreamConnection.h"

namespace PokemonAutomation{
namespace PABotBase2{




class DeviceHandle{
public:
    DeviceHandle(StreamConnection& connection)
        : m_connection(connection)
        , m_device_protocol(0)
        , m_command_queue_size(4)
    {}

    void connect();


private:
    StreamConnection& m_connection;

    uint32_t m_device_protocol;
    uint8_t m_command_queue_size;

};



}
}
#endif
