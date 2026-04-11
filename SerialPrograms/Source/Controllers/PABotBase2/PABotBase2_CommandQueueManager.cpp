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

void CommandQueueManager::wait_for_all(){
    std::unique_lock<Mutex> lg(m_lock);
    while (!m_pending_commands.empty()){
        throw_if_cancelled();
        m_cv.wait(lg);
    }
}
void CommandQueueManager::wait_for_command_finish(uint8_t id){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        throw_if_cancelled();

        //  Command doesn't exist.
        auto iter = m_pending_commands.find(id);
        if (iter == m_pending_commands.end()){
            return;
        }

        m_cv.wait(lg);
    }
}

bool CommandQueueManager::send_cancel(WallDuration timeout){
    MessageHeader message;
    message.message_bytes = sizeof(MessageHeader);
    message.opcode = PABB2_MESSAGE_OPCODE_CQ_CANCEL;
    message.id = 0;
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_pending_commands.clear();
    }
    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &message);
    WallClock start = current_time();
    size_t bytes_sent = m_connection.reliable_send(&message, message.message_bytes, timeout);
    WallClock end = current_time();
    m_cv.notify_all();

    if (timeout == WallDuration::max()){
        return bytes_sent == message.message_bytes;
    }

    if (bytes_sent == message.message_bytes){
        m_logger.log(
            "CommandQueueManager(): Issuing non-blocking cancel... " +
            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) +
            " ms",
            COLOR_BLUE
        );
        return true;
    }else{
        m_logger.log(
            "CommandQueueManager(): Issuing non-blocking cancel... " +
            std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()) +
            " ms (timed out)",
            COLOR_RED
        );
        return false;
    }
}
void CommandQueueManager::send_replace_on_next(){
    MessageHeader message;
    message.message_bytes = sizeof(MessageHeader);
    message.opcode = PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT;
    message.id = 0;
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_pending_commands.clear();
    }
    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &message);
    m_connection.reliable_send(&message, message.message_bytes);
    m_cv.notify_all();
}


uint8_t CommandQueueManager::send_command(MessageHeader& command){
    {
        std::unique_lock<Mutex> lg(m_lock);
        while (true){
            throw_if_cancelled();

            if (m_pending_commands.size() >= m_command_queue_size){
                m_cv.wait(lg);
                continue;
            }

            command.id = m_command_seqnum;

            //  Wait until the slot is available.
            auto iter = m_pending_commands.find(command.id);
            if (iter != m_pending_commands.end()){
                m_cv.wait(lg);
                continue;
            }

            m_pending_commands.emplace(
                command.id,
                std::make_shared<CommandHandle>()
            );
            m_command_seqnum++;
            break;
        }
    }
    m_message_loggers.log_send(m_logger, GlobalSettings::instance().LOG_EVERYTHING, &command);
    m_connection.reliable_send(&command, command.message_bytes);
    m_cv.notify_all();
    return command.id;
}
void CommandQueueManager::report_command_finished(const MessageHeader& finished_message){
    {
        std::lock_guard<Mutex> lg(m_lock);
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
    }
    m_cv.notify_all();
}




}
}
