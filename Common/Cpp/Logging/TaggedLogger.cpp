/*  Tagged Logger
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Time.h"
#include "TaggedLogger.h"

namespace PokemonAutomation{


TaggedLogger::TaggedLogger(Logger& logger, std::string tag)
    : m_logger(logger)
{
    m_tags.emplace_back(std::move(tag));
}
TaggedLogger::TaggedLogger(TaggedLogger& logger, std::string tag)
    : m_logger(logger.base_logger())
    , m_tags(logger.m_tags)
{
    m_tags.emplace_back(std::move(tag));
}

std::string TaggedLogger::add_timestamp_and_tags(const std::string& msg) const{
    std::string str = current_time_to_str() + " - ";
    for (const std::string& tag : m_tags){
        str += "[" + tag + "]";
    }
    str += ": " + msg;
    return str;
}

void TaggedLogger::log(const std::string& msg, Color color){
    m_logger.log(add_timestamp_and_tags(msg), color);
}
void TaggedLogger::log(const std::string& msg, Color color, LogLevel level){
    m_logger.log(add_timestamp_and_tags(msg), color, level);
}


}
