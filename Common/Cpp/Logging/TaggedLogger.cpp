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
    , m_tag(std::move(tag))
{}

void TaggedLogger::log(const std::string& msg, Color color){
    std::string str =
        current_time_to_str() +
        " - [" + m_tag + "]: " +
        msg;
    m_logger.log(std::move(str), color);
}


}
