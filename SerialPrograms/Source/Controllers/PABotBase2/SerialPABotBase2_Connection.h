/*  Serial Port (PABotBase2) Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase2_Connection_H
#define PokemonAutomation_Controllers_SerialPABotBase2_Connection_H

#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/SerialConnection/SerialConnection.h"
#include "Common/PABotBase2/ReliableConnectionLayer/PABotBase2CC_ReliableStreamConnection.h"
#include "Controllers/PABotBase2/PABotBase2_Connection.h"
#include "Controllers/SerialPABotBase/Connection/MessageLogger.h"

namespace PokemonAutomation{
namespace SerialPABotBase{



class SerialPABotBase2_Connection final : public PABotBase2::Connection{
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
    bool open_device_connection(bool set_to_null_controller);
    void connect_thread_body(bool set_to_null_controller);


private:
    SerialLogger m_logger;
    std::string m_device_name;

//    Mutex m_lock;
    AsyncTask m_connect_thread;
    std::unique_ptr<SerialConnection> m_unreliable_connection;
    std::unique_ptr<PABotBase2::ReliableStreamConnection> m_stream_connection;
};




}
}
#endif
