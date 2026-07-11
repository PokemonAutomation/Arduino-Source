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
    virtual void log(std::string&& msg, Color color = Color()){
        m_listeners.run_method(
            (void (Logger::*)(std::string&&, Color))&Logger::log,
            std::move(msg), color
        );
    }
    virtual void log(const char* msg, Color color = Color()){
        m_listeners.run_method(
            (void (Logger::*)(const char* msg, Color))&Logger::log,
            msg, color
        );
    }

    virtual std::vector<std::string> get_last(){
        std::vector<std::string> ret;
        m_listeners.run_on_all([&](Logger& logger){
            ret = logger.get_last();
            return !ret.empty();
        });
        return ret;
    }


private:
    ListenerSet<Logger> m_listeners;
};



}
#endif
