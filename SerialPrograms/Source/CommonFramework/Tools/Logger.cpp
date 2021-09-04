/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Libraries/Logging.h"
#include "CommonFramework/PersistentSettings.h"
#include "Logger.h"

namespace PokemonAutomation{

TaggedLogger::TaggedLogger(Logger& logger, std::string tag)
    : m_logger(logger)
    , m_tag(std::move(tag))
{}

void TaggedLogger::log(const char* msg, QColor color){
    log(std::string(msg), color);
}
void TaggedLogger::log(const std::string& msg, QColor color){
    std::string str =
        current_time() +
        " - [" + m_tag + "]: " +
        msg;
    m_logger.log(str, color);
}
void TaggedLogger::log(const QString& msg, QColor color){
    log(msg.toUtf8().toStdString(), color);
}



SerialLogger::SerialLogger(Logger& logger, std::string tag)
    : TaggedLogger(logger, std::move(tag))
    , PokemonAutomation::MessageLogger(PERSISTENT_SETTINGS().log_everything)
{}
void SerialLogger::log(std::string msg){
    TaggedLogger::log(msg, "green");
}



}

