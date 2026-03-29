/*  PABotBase2 Command Queue
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/PABotBase2/PABotBase2CC_MessageDumper.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "PABotBase2_CommandQueueManager.h"

namespace PokemonAutomation{
namespace PABotBase2{


void CommandQueueManager::set_command_queue_size(uint8_t command_queue_size){
    {
        std::unique_lock<Mutex> lg(m_lock);
        m_command_queue_size = command_queue_size;
    }
    m_cv.notify_all();
}


void CommandQueueManager::wait_for_all(){
    std::unique_lock<Mutex> lg(m_lock);
    while (!m_pending_commands.empty()){
        m_scope.throw_if_cancelled();
        m_cv.wait(lg);
    }
}
void CommandQueueManager::wait_for_command_finish(uint8_t id){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        m_scope.throw_if_cancelled();

        //  Command doesn't exist.
        auto iter = m_pending_commands.find(id);
        if (iter == m_pending_commands.end()){
            return;
        }

        if (iter->second.empty()){
            m_cv.wait(lg);
            continue;
        }

        m_pending_commands.erase(iter);
        return;
    }
}

void CommandQueueManager::send_cancel(){
    m_pending_commands.clear();
    MessageHeader message;
    message.message_bytes = sizeof(MessageHeader);
    message.opcode = PABB2_MESSAGE_OPCODE_CQ_CANCEL;
    message.id = 0;
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("[MLC]: Sending: " + tostr(&message), COLOR_DARKGREEN);
    }
    m_connection.reliable_send(&message, message.message_bytes);
}
void CommandQueueManager::send_replace_on_next(){
    m_pending_commands.clear();
    MessageHeader message;
    message.message_bytes = sizeof(MessageHeader);
    message.opcode = PABB2_MESSAGE_OPCODE_CQ_REPLACE_ON_NEXT;
    message.id = 0;
    if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("[MLC]: Sending: " + tostr(&message), COLOR_DARKGREEN);
    }
    m_connection.reliable_send(&message, message.message_bytes);
}


uint8_t CommandQueueManager::send_command(MessageHeader& command){
    std::unique_lock<Mutex> lg(m_lock);
    while (true){
        m_scope.throw_if_cancelled();

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

        m_pending_commands[command.id];
        m_command_seqnum++;
        break;
    }

    auto iter = m_message_handlers.find(command.opcode);
    if (iter != m_message_handlers.end()){
        if (iter->second->should_print()){
            m_logger.log("[MLC]: Sending: " + iter->second->tostr(&command), COLOR_DARKGREEN);
        }
    }else if (GlobalSettings::instance().LOG_EVERYTHING){
        m_logger.log("[MLC]: Sending: " + tostr(&command), COLOR_DARKGREEN);
    }

    m_connection.reliable_send(&command, command.message_bytes);

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
        iter->second = std::string((const char*)&finished_message, finished_message.message_bytes);
    }
    m_cv.notify_all();
}




}
}
