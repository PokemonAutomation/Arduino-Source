/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "ClientSource/Libraries/Logging.h"
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




std::string combine_string(const char* a, const char* b){
    std::string str(a);
    str += ": ";
    str += b;
    return str;
}
std::string combine_string(const char* a, const std::string& b){
    std::string str(a);
    str += ": ";
    str += b;
    return str;
}
QString combine_string(const char* a, const QString& b){
    QString str(a);
    str += ": ";
    str += b;
    return str;
}




}

