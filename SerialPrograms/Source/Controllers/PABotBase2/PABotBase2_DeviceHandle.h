/*  PABotBase2 Device Handle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H
#define PokemonAutomation_Controllers_PABotBase2_DeviceHandle_H

#include <string.h>
#include <deque>
#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/StreamConnections/PushingStreamConnections.h"
#include "Common/PABotBase2/PABotBase2_MessageProtocol.h"
#include "Common/PABotBase2/PABotBase2CC_MessageDumper.h"
#include "Controllers/ControllerTypes.h"
#include "PABotBase2_CommandQueueManager.h"
#include "PABotBase2_MessageHandler.h"

namespace PokemonAutomation{
namespace PABotBase2{



class DeviceHandle final : public CancellableScope, private StreamListener{
public:
    DeviceHandle(
        CancellableScope* parent,
        Logger& logger,
        ReliableStreamConnectionPushing& connection
    );
    virtual ~DeviceHandle();

    Logger& logger() const{
        return m_logger;
    }
    MessageLogger& message_logger(){
        return m_message_loggers;
    }
    ReliableStreamConnectionPushing& connection(){
        return m_connection;
    }

    template <typename MessageHandler, class... Args>
    void add_message_handler(Args&&... args){
        auto ret = m_message_handlers.emplace(
            MessageHandler::OPCODE,
            std::make_unique<MessageHandler>(std::forward<Args>(args)...)
        );
        if (!ret.second){
            throw InternalProgramError(
                &m_logger, PA_CURRENT_FUNCTION,
                "Duplicate Message Opcode: " + std::to_string(MessageHandler::OPCODE)
            );
        }
    }

    virtual bool cancel(std::exception_ptr exception) noexcept override;

    void connect();
    void try_set_controller_type(
        ControllerType controller_type,
        bool clear_settings
    ) noexcept;


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

    CommandQueueManager& command_queue(){
        return m_command_queue;
    }


public:
    ControllerType refresh_controller_type();

    uint8_t send_request(MessageHeader& request);
    std::optional<uint8_t> try_send_request(MessageHeader& request, WallDuration timeout) noexcept;
    std::string wait_for_request_response(uint8_t id);

    template <typename ResponseType, uint8_t response_opcode>
    void wait_for_request_response(ResponseType& response, uint8_t id){
        std::string raw = wait_for_request_response(id);
        const MessageHeader* header = (const MessageHeader*)raw.data();
        if (header->opcode != response_opcode){
            throw SerialProtocolException(
                m_logger, PA_CURRENT_FUNCTION,
                "Received Incorrect Response Type: Expected = " + std::to_string(response_opcode) +
                ", Actual = " + std::to_string(header->opcode)
            );
        }
        if (header->message_bytes != sizeof(ResponseType)){
            throw SerialProtocolException(
                m_logger, PA_CURRENT_FUNCTION,
                "Received Incorrect Response Size: Expected = " + std::to_string(sizeof(ResponseType)) +
                ", Actual = " + std::to_string(header->message_bytes)
            );
        }
        memcpy(&response, header, sizeof(ResponseType));
    }



private:
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
    CommandQueueManager m_command_queue;

    uint32_t m_device_protocol = 0;
    uint32_t m_device_id = 0;
    std::string m_device_name;
    uint32_t m_device_firmware_version = 0;
    std::vector<ControllerType> m_controller_list;

//    std::atomic<ControllerType> m_controller_type;

    uint8_t m_request_seqnum = 0;

    Mutex m_lock;
    ConditionVariable m_cv;
    std::map<uint8_t, std::string> m_pending_requests;

    std::deque<char> m_buffer;

    MessageLogger m_message_loggers;

    struct MessageConverter{
        bool always_print = false;
        std::string (*tostr)(const MessageHeader*);
    };
    std::map<uint8_t, std::unique_ptr<MessageHandler>> m_message_handlers;
};







}
}
#endif
