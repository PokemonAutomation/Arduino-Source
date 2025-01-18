/*  Serial Port (PABotBase) Option
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
 *
 *
 *  This class isn't fully thread-safe like the rest of the session classes.
 *  TODO: This will be fixed in the future and will require a rewrite of BotBaseHandle.
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_Session_H
#define PokemonAutomation_Controllers_SerialPABotBase_Session_H

#include <set>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "SerialPABotBase_Handle.h"
#include "SerialPABotBase_Option.h"

namespace PokemonAutomation{


class SerialPortSession{
public:
    struct Listener{
        virtual void on_ready(bool ready) = 0;
    };

    void add_listener(Listener& listener);
    void remove_listener(Listener& listener);


public:
    ~SerialPortSession();
    SerialPortSession(Logger& logger, SerialPortOption& option);

    QSerialPortInfo get() const;
    void set(QSerialPortInfo port);
    void set(const QSerialPortInfo* port);

public:
//    SerialPortOption* operator->(){ return &m_option; }
//    SerialPortOption& option(){ return m_option; }

    bool is_ready() const{ return m_connection.state() == BotBaseHandle::State::READY; }
    BotBaseHandle& botbase(){ return m_connection; }
    PABotBaseLevel min_pabotbase() const{ return m_connection.min_pabotbase(); }

    void stop();
    void reset();


private:
    void push_ready(bool ready);


private:
    SerialPortOption& m_option;
    SerialLogger m_logger;
    BotBaseHandle m_connection;

    mutable std::mutex m_lock;
    std::set<Listener*> m_listeners;

    LifetimeSanitizer m_sanitizer;
};




}
#endif
