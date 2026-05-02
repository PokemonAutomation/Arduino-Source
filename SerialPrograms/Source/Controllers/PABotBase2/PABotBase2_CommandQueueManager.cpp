/*  PABotBase2 Command Queue
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "Common/PABotBase2/PABotBase2CC_MessageDumper.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "PABotBase2_CommandQueueManager.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{


void CommandQueueManager::set_command_queue_size(uint8_t command_queue_size){
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_command_queue_size = command_queue_size;
    }
    m_cv.notify_all();
}
bool CommandQueueManager::cancel(std::exception_ptr exception) noexcept{
    bool ret = Cancellable::cancel(std::move(exception));
    {
        std::lock_guard<Mutex> lg(m_lock);
#if 0
        try{
            send_cancel(std::chrono::milliseconds(100));
        }catch (...){}
#endif
    }
    m_cv.notify_all();
    return ret;
}

void CommandQueueManager::wait_for_all(Cancellable* cancellable){
    std::unique_lock<Mutex> lg(m_lock);
    while (!m_pending_commands.empty()){
        throw_if_cancelled(cancellable);
        cv_wait(cancellable, lg);
    }
}
void CommandQueueManager::wait_for_command_finish(Cancellable* cancellable, uint8_t id){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        throw_if_cancelled(cancellable);

        //  Command doesn't exist.
        auto iter = m_pending_commands.find(id);
        if (iter == m_pending_commands.end()){
            return;
        }

        cv_wait(cancellable, lg);
    }
}

void CommandQueueManager::send_cancel() noexcept{
    bool success;
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_pending_special = PABB2_MESSAGE_OPCODE_CQ_CANCEL;
        success = try_push_pending_specials();
    }
    if (success){
        m_cv.notify_all();
    }
}
void CommandQueueManager::send_replace_on_next() noexcept{
    bool success;
    {
        std::unique_lock<Mutex> lg(m_lock);
        if (m_pending_special != PABB2_MESSAGE_OPCODE_CQ_CANCEL){
            m_pending_special = PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT;
        }
        success = try_push_pending_specials();
    }
    if (success){
        m_cv.notify_all();
    }
}


uint8_t CommandQueueManager::send_command(Cancellable* cancellable, MessageHeader& command){
    {
        bool need_to_wait = false;
        std::unique_lock<Mutex> lg(m_lock);
        try_push_pending_specials();
        while (true){
            if (need_to_wait){
                cv_wait(cancellable, lg);
            }
            need_to_wait = true;
            throw_if_cancelled(cancellable);

            if (m_pending_commands.size() >= m_command_queue_size){
                continue;
            }

//            cout << "Send: " << (unsigned)m_command_seqnum << ", queue size = " << m_pending_commands.size() << endl;
            command.id = m_command_seqnum;

            //  Wait until the slot is available.
            auto iter = m_pending_commands.find(command.id);
            if (iter != m_pending_commands.end()){
                continue;
            }

            iter = m_pending_commands.emplace(
                command.id,
                std::make_shared<CommandHandle>()
            ).first;

            m_lock.unlock();
            bool sent = m_connection.reliable_send_all_or_nothing(
                &command, command.message_bytes,
                WallDuration::max()
            );
            m_lock.lock();

            if (sent){
                m_command_seqnum++;
                break;
            }
            m_pending_commands.erase(iter);
        }
    }
//    cout << "Post send 0: " << (unsigned)command.id << endl;
    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &command);
//    cout << "Post send 1: " << (unsigned)command.id << endl;
    m_cv.notify_all();
    return command.id;
}
void CommandQueueManager::report_command_finished(const MessageHeader& finished_message){
    {
        std::lock_guard<Mutex> lg(m_lock);
//        cout << "Done: " << (unsigned)finished_message.id << endl;
        auto iter = m_pending_commands.find(finished_message.id);
        if (iter == m_pending_commands.end()){
            m_logger.log("[MLC]: Received command finish for unknown ID: " + std::to_string(finished_message.id));
            return;
        }
        iter->second->finished = true;
        memcpy(
            &iter->second->device_timestamp,
            &((const Message_u32&)finished_message).data,
            sizeof(uint32_t)
        );
        m_pending_commands.erase(iter);
        try_push_pending_specials();
    }
    m_cv.notify_all();
}


bool CommandQueueManager::try_push_pending_specials() noexcept{
    //  Must call under lock.
    if (m_pending_special == PABB2_MESSAGE_OPCODE_INVALID){
        return false;
    }

    MessageHeader message;
    message.message_bytes = sizeof(MessageHeader);
    message.opcode = m_pending_special;
    message.id = 0;

    m_lock.unlock();
    bool sent = m_connection.reliable_send_all_or_nothing(
        &message, message.message_bytes,
        WallDuration::zero()
    );
    m_lock.lock();

    if (!sent){
        return false;
    }

    m_pending_special = PABB2_MESSAGE_OPCODE_INVALID;
    m_pending_commands.clear();

    try{
        m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &message);
    }catch (...){}
    return true;
}


void CommandQueueManager::on_cancellable_cancel(
    Cancellable& cancellable,
    std::exception_ptr reason
){
    {
        std::unique_lock<Mutex> lg(m_lock);
    }
    m_cv.notify_all();
}
void CommandQueueManager::cv_wait(Cancellable* cancellable, std::unique_lock<Mutex>& lg){
    if (cancellable == nullptr){
        m_cv.wait(lg);
        return;
    }

    cancellable->add_cancel_listener(*this);
    m_cv.wait(lg);

    //  Unlock to remove. Otherwise, it may deadlock with "on_cancellable_cancel()"
    //  being called from a listener callback.
    lg.unlock();
    cancellable->remove_cancel_listener(*this);
    lg.lock();
}
void CommandQueueManager::throw_if_cancelled(Cancellable* cancellable){
    Cancellable::throw_if_cancelled();
    if (cancellable){
        cancellable->throw_if_cancelled();
    }
}



}
}
