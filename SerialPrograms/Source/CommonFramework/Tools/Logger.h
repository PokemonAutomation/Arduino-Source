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

namespace PokemonAutomation{


class Logger{
public:
    virtual void log(const char* msg, QColor color = QColor()) = 0;
    virtual void log(const std::string& msg, QColor color = QColor()) = 0;
    virtual void log(const QString& msg, QColor color = QColor()) = 0;
};


}
#endif

