/*  Multi-Output Logger
 *
 *  From: https://github.com/PokemonAutomation/
 */

#ifndef PokemonAutomation_Logging_MultiOutputLogger_H
#define PokemonAutomation_Logging_MultiOutputLogger_H

#include "Common/Cpp/ListenerSet.h"
#include "AbstractLogger.h"

namespace PokemonAutomation{



class MultiOutputLogger : public Logger{
public:
    MultiOutputLogger()
        : m_listeners(true)
    {}

    void add_listener(Logger& logger){
        m_listeners.add(logger);
    }
    void remove_listener(Logger& logger){
        m_listeners.remove(logger);
    }

public:
    virtual void log(const std::string& msg, Color color = Color()){
        m_listeners.run_method(
            (void (Logger::*)(const std::string&, Color))&Logger::log,
            msg, color
        );
    }
    virtual void log(const char* msg, Color color = Color()){
        m_listeners.run_method(
            (void (Logger::*)(const char* msg, Color))&Logger::log,
            msg, color
        );
    }


private:
    ListenerSet<Logger> m_listeners;
};



}
#endif
