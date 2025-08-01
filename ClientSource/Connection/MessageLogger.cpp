/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "ClientSource/Libraries/MessageConverter.h"
#include "BotBaseMessage.h"
#include "MessageLogger.h"

namespace PokemonAutomation{


//void MessageLogger::log(std::string msg){
//    log(msg);
//}
void MessageLogger::on_send(const BotBaseMessage& message, bool is_retransmit){
    bool print = false;
    do{
        if (is_retransmit){
            print = true;
        }
        if (PABB_MSG_IS_REQUEST(message.type)){
            print = true;
        }
        if (PABB_MSG_IS_COMMAND(message.type)){
            print = true;
        }
        if (message.type == PABB_MSG_REQUEST_CLOCK){
            print = false;
        }
#if 0
        if (message.type == PABB_MSG_CONTROLLER_STATE){
//            pabb_controller_state body;
//            memcpy(&body, message.body.c_str(), sizeof(pabb_controller_state));
//            print = body.ticks >= 5;
            print = false;
        }
#endif

        if (m_log_everything.load(std::memory_order_relaxed)){
            print = true;
        }

    }while (false);
    if (!print){
        return;
    }
    std::string str = message_to_string(message);
    if (str.empty()){
        return;
    }
    if (is_retransmit){
        log("Re-Send: " + str, COLOR_DARKGREEN);
    }else{
        log("Sending: " + str, COLOR_DARKGREEN);
    }
}
void MessageLogger::on_recv(const BotBaseMessage& message){
    bool print = false;
    do{
        if (PABB_MSG_IS_ERROR(message.type)){
            print = true;
        }
        if (PABB_MSG_IS_INFO(message.type)){
            print = true;
        }

        if (m_log_everything.load(std::memory_order_relaxed)){
            print = true;
        }

    }while (false);
    if (!print){
        return;
    }
    log("Receive: " + message_to_string(message), COLOR_DARKGREEN);
}



SerialLogger::SerialLogger(Logger& logger, bool log_everything)
    : MessageLogger(log_everything)
    , m_logger(logger)
    , m_history(200)
{}
void SerialLogger::log(const char* msg, Color color){
    if (ok_to_log()){
        m_logger.log(msg, color);
    }
}
void SerialLogger::log(const std::string& msg, Color color){
    if (ok_to_log()){
        m_logger.log(msg, color);
    }
}

bool SerialLogger::ok_to_log(){
    WallClock now = current_time();
    WriteSpinLock lg(m_lock);
    while (!m_history.empty()){
        if (now - m_history.front() < std::chrono::seconds(1)){
            break;
        }
        m_history.pop_front();
    }
    if (!m_history.try_push_back(now)){
        m_messages_dropped++;
        return false;
    }

    if (m_messages_dropped != 0){
        m_logger.log("Dropped " + tostr_u_commas(m_messages_dropped) + " message(s) due to logging rate limit.", COLOR_RED);
        m_messages_dropped = 0;
    }

    return true;
}



}
