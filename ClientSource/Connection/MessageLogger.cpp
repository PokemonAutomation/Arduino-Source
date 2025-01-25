/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_Protocol_PushButtons.h"
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
#if 1
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
        log("Re-Send: " + str);
    }else{
        log("Sending: " + str);
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
    log("Receive: " + message_to_string(message));
}



SerialLogger::SerialLogger(Logger& logger, bool log_everything)
    : MessageLogger(log_everything)
    , m_logger(logger)
{}
void SerialLogger::log(const char* msg, Color color){
    m_logger.log(msg, color);
}
void SerialLogger::log(const std::string& msg, Color color){
    m_logger.log(msg, color);
}
void SerialLogger::log(std::string msg){
    m_logger.log(msg, COLOR_DARKGREEN);
}




}
