/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H
#define PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H

#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/StreamConnections/StreamConnection.h"
#include "Common/PABotBase2/DataLayer/PABotBase2_MessageProtocol.h"

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
        PokemonAutomation::StreamConnection& connection
    );
    virtual ~DeviceHandle();

    virtual bool cancel(std::exception_ptr exception) noexcept override;

    void connect();


private:
    virtual void on_recv(const void* data, size_t bytes) override;

    void send_data(const void* data, size_t bytes);
    void send_request(pabb2_MessageHeader_Request& request);
    std::string wait_for_response(uint8_t id);

    uint32_t query_u32(uint8_t opcode);


private:
    Logger& m_logger;
    PokemonAutomation::StreamConnection& m_connection;

    uint32_t m_device_protocol = 0;
    uint32_t m_device_id = 0;
    uint32_t m_device_firmware_version = 0;
    uint8_t m_command_queue_size = 4;

    uint8_t m_seqnum = 0;

    struct LiveRequest{
        Mutex lock;
        ConditionVariable cv;
        std::string response;
    };

    Mutex m_lock;
    ConditionVariable m_cv;
    std::map<uint8_t, LiveRequest> m_pending_requests;
};



}
}
#endif
