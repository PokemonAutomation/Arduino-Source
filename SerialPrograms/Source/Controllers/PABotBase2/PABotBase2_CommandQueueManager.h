/*  PABotBase2 Command Queue
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_PABotBase2_CommandQueue_H
#define PokemonAutomation_Controllers_PABotBase2_CommandQueue_H

#include <map>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/StreamConnections/PushingStreamConnections.h"
#include "Common/PABotBase2/PABotBase2_MessageProtocol.h"
#include "PABotBase2_MessageHandler.h"

namespace PokemonAutomation{
namespace PABotBase2{



class CommandQueueManager{
public:
    CommandQueueManager(
        Logger& logger,
        Cancellable& scope,
        ReliableStreamConnectionPushing& connection,
        const std::map<uint8_t, std::unique_ptr<MessageHandler>>& message_handlers
    )
        : m_logger(logger)
        , m_scope(scope)
        , m_connection(connection)
        , m_message_handlers(message_handlers)
    {}

    void set_command_queue_size(uint8_t command_queue_size);


public:
    void wait_for_all();
    void wait_for_command_finish(uint8_t id);

    void send_cancel();
    void send_replace_on_next();

    uint8_t send_command(MessageHeader& command);
    void report_command_finished(const MessageHeader& finished_message);


private:
    Logger& m_logger;
    Cancellable& m_scope;
    ReliableStreamConnectionPushing& m_connection;
    const std::map<uint8_t, std::unique_ptr<MessageHandler>>& m_message_handlers;

    Mutex m_lock;
    ConditionVariable m_cv;
    uint8_t m_command_queue_size = 4;
    uint8_t m_command_seqnum = 0;

    std::map<uint8_t, std::string> m_pending_commands;
};




}
}
#endif
