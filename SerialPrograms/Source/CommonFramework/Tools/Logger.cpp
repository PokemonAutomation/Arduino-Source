/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Libraries/Logging.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Logger.h"

namespace PokemonAutomation{


Logger& global_logger_tagged(){
    static TaggedLogger logger(global_logger_raw(), "Global");
    return logger;
}


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
    , PokemonAutomation::MessageLogger(GlobalSettings::instance().LOG_EVERYTHING)
{}
void SerialLogger::log(std::string msg){
    TaggedLogger::log(msg, "green");
}



}

