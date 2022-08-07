/*  Serial Port Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      SerialPortSession represents a live serial port. It holds onto a serial
 *  port object and can be configured, reset, etc...
 *
 *  This class is not responsible for any UI. However, any changes made to this
 *  class will be forwarded to any UI components that are attached to it.
 *
 *  The UI that allows a user to control this class is in SerialPortWidget.
 *
 */

#ifndef PokemonAutomation_SerialPortSession_H
#define PokemonAutomation_SerialPortSession_H

#include <set>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/Tools/BotBaseHandle.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "SerialPortOption.h"

namespace PokemonAutomation{


class SerialPortSession{
public:
    struct Listener{
        virtual void on_ready(bool ready) = 0;
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    SerialPortSession(Logger& logger, SerialPortOption& option);
    ~SerialPortSession();


public:
    SerialPortOption* operator->(){ return &m_option; }
    SerialPortOption& option(){ return m_option; }

    bool is_ready() const{ return m_connection.state() == BotBaseHandle::State::READY; }
    BotBaseHandle& botbase(){ return m_connection; }

    void stop();
    void reset();


private:
    void push_ready(bool ready);


private:
    SerialPortOption& m_option;
    SerialLogger m_logger;
    BotBaseHandle m_connection;

    std::mutex m_lock;
    std::set<Listener*> m_listeners;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
