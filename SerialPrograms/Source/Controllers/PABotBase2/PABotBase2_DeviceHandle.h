/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H
#define PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H

#include <deque>
#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/StreamConnections/PushingStreamConnections.h"
#include "Common/PABotBase2/DataLayer/PABotBase2_MessageProtocol.h"
#include "Controllers/ControllerTypes.h"

namespace PokemonAutomation{
namespace PABotBase2{




class DeviceHandle final
    : public CancellableScope
    , private StreamListener
{
public:
    DeviceHandle(
        CancellableScope* parent,
        Logger& logger,
        ReliableStreamConnectionPushing& connection
    );
    virtual ~DeviceHandle();

    virtual bool cancel(std::exception_ptr exception) noexcept override;

    void connect();


public:
    uint32_t device_firmware_version() const{
        return m_device_firmware_version;
    }
    const std::string& device_name() const{
        return m_device_name;
    }
    const std::vector<ControllerType>& controller_list() const{
        return m_controller_list;
    }


private:
    void send_request(pabb2_MessageHeader& request);
    std::string wait_for_response(uint8_t id);

    uint32_t query_u32(uint8_t opcode);
    std::string query_data(uint8_t opcode);

    void throw_incompatible_protocol();
    void query_protocol();
    void query_controller_list();
    void query_command_queue();

    virtual void on_recv(const void* data, size_t bytes) override;

private:
    Logger& m_logger;
    ReliableStreamConnectionPushing& m_connection;

    uint32_t m_device_protocol = 0;
    uint32_t m_device_id = 0;
    std::string m_device_name;
    uint32_t m_device_firmware_version = 0;
    std::vector<ControllerType> m_controller_list;
    uint8_t m_command_queue_size = 4;

    uint8_t m_seqnum = 0;

    struct LiveRequest{
        ConditionVariable cv;
        std::string response;
    };

    Mutex m_lock;
    ConditionVariable m_cv;
    std::map<uint8_t, LiveRequest> m_pending_requests;

    std::deque<char> m_buffer;
};





}
}
#endif
