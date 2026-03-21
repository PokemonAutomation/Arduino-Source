/*  Serial Port (PABotBase2) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase2_Connection_H
#define PokemonAutomation_Controllers_SerialPABotBase2_Connection_H

#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/SerialConnection/SerialConnection.h"
#include "Common/Cpp/StreamConnections/ReliableStreamConnection.h"
#include "Controllers/SerialPABotBase/Connection/MessageLogger.h"
#include "Controllers/ControllerConnection.h"
#include "PABotBase2_DeviceHandle.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class SerialPABotBase2_Connection final : public ControllerConnection, public CancellableScope{
public:
    SerialPABotBase2_Connection(
        Logger& logger,
        std::string name,
        bool set_to_null_controller
    );
    ~SerialPABotBase2_Connection();


public:
    ControllerType refresh_controller_type();


private:
    bool open_serial_port();
    bool open_serial_connection();
    bool open_device_connection();
    void connect_thread_body(bool set_to_null_controller);


private:
    SerialLogger m_logger;
    std::string m_device_name;

    AsyncTask m_connect_thread;
    std::unique_ptr<SerialConnection> m_unreliable_connection;
    std::unique_ptr<ReliableStreamConnection> m_stream_connection;
    std::unique_ptr<PABotBase2::DeviceHandle> m_device;
};




}
}
#endif
