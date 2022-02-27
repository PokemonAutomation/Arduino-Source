/*  Abstract Logger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AbstractLogger_H
#define PokemonAutomation_AbstractLogger_H

#include <string>
#include "Color.h"

namespace PokemonAutomation{


class Logger{
public:
    virtual void log(const char* msg, Color color = Color()) = 0;
    virtual void log(const std::string& msg, Color color = Color()) = 0;
};


}
#endif
