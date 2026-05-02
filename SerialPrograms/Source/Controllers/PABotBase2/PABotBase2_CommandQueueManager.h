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
#include "Common/PABotBase2/PABotBase2CC_MessageDumper.h"
#include "PABotBase2_MessageHandler.h"

namespace PokemonAutomation{
namespace PABotBase2{



class CommandQueueManager final : public Cancellable, public Cancellable::CancelListener{
public:
    CommandQueueManager(
        Logger& logger,
        CancellableScope& scope,
        ReliableStreamConnectionPushing& connection,
        const MessageLogger& message_loggers
    )
        : m_logger(logger)
        , m_connection(connection)
        , m_message_loggers(message_loggers)
    {
        attach(scope);
    }
    ~CommandQueueManager(){
        detach();
    }

    void set_command_queue_size(uint8_t command_queue_size);

    virtual bool cancel(std::exception_ptr exception) noexcept override;


public:
    void wait_for_all(Cancellable* cancellable);
    void wait_for_command_finish(Cancellable* cancellable, uint8_t id);

    void send_cancel() noexcept;
    void send_replace_on_next() noexcept;

    uint8_t send_command(Cancellable* cancellable, MessageHeader& command);
    void report_command_finished(const MessageHeader& finished_message);


private:
    bool try_push_pending_specials() noexcept;

    virtual void on_cancellable_cancel(
        Cancellable& cancellable,
        std::exception_ptr reason
    ) override;
    void cv_wait(Cancellable* cancellable, std::unique_lock<Mutex>& lg);
    void throw_if_cancelled(Cancellable* cancellable);


private:
    Logger& m_logger;
    ReliableStreamConnectionPushing& m_connection;
    const MessageLogger& m_message_loggers;

    mutable Mutex m_lock;
    ConditionVariable m_cv;
    uint8_t m_command_queue_size = 4;
    uint8_t m_command_seqnum = 0;

    uint8_t m_pending_special = PABB2_MESSAGE_OPCODE_INVALID;

    SpinLock m_pending_commands_lock;
    struct CommandHandle{
        bool finished = false;
        uint32_t device_timestamp = 0;
    };
    std::map<uint8_t, std::shared_ptr<CommandHandle>> m_pending_commands;
};




}
}
#endif
