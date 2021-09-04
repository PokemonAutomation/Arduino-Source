/*  Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Logger_H
#define PokemonAutomation_Logger_H

#include <string>
#include <QString>
#include <QColor>
#include "ClientSource/Libraries/Logging.h"

namespace PokemonAutomation{


class Logger{
public:
    virtual void log(const char* msg, QColor color = QColor()) = 0;
    virtual void log(const std::string& msg, QColor color = QColor()) = 0;
    virtual void log(const QString& msg, QColor color = QColor()) = 0;
};


Logger& global_logger();


class TaggedLogger : public Logger{
public:
    TaggedLogger(Logger& logger, std::string tag);

    Logger& base_logger(){ return m_logger; }

    void log(const char* msg, QColor color = QColor()) override;
    void log(const std::string& msg, QColor color = QColor()) override;
    void log(const QString& msg, QColor color = QColor()) override;

private:
    Logger& m_logger;
    std::string m_tag;
};


class SerialLogger : public TaggedLogger, public PokemonAutomation::MessageLogger{
public:
    SerialLogger(Logger& logger, std::string tag);
    using TaggedLogger::log;
    virtual void log(std::string msg) override;
};


}
#endif

