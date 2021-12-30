/*  Device Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "DeviceLogger.h"

namespace PokemonAutomation{


SerialLogger::SerialLogger(Logger& logger, std::string tag)
    : TaggedLogger(logger, std::move(tag))
    , PokemonAutomation::MessageLogger(GlobalSettings::instance().LOG_EVERYTHING)
{}
void SerialLogger::log(std::string msg){
    TaggedLogger::log(msg, COLOR_DARKGREEN);
}



}
