/*  Device Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "DeviceLogger.h"

namespace PokemonAutomation{


SerialLogger::SerialLogger(Logger& logger)
    : PokemonAutomation::MessageLogger(GlobalSettings::instance().LOG_EVERYTHING)
    , m_logger(logger)
{}
void SerialLogger::log(const char* msg, Color color){
    m_logger.log(msg, color);
}
void SerialLogger::log(const std::string& msg, Color color){
    m_logger.log(msg, color);
}
void SerialLogger::log(const QString& msg, Color color){
    m_logger.log(msg, color);
}
void SerialLogger::log(std::string msg){
    m_logger.log(msg, COLOR_DARKGREEN);
}



}
