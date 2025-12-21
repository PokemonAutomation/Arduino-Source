/*  Message Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "MessageLogger.h"

namespace PokemonAutomation{



SerialLogger::SerialLogger(Logger& logger, bool log_everything)
    : m_logger(logger)
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
        m_logger.log(std::format("Dropped {:L} message(s) due to logging rate limit.", m_messages_dropped), COLOR_RED);
        m_messages_dropped = 0;
    }

    return true;
}



}
