/*  Controller Connection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Controllers_ControllerConnection_H
#define PokemonAutomation_Controllers_ControllerConnection_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "ControllerDescriptor.h"

namespace PokemonAutomation{


class ControllerConnection{
public:
    struct StatusListener{
        virtual void ready_changed(bool ready) = 0;
        virtual void status_text_changed(const std::string& text) = 0;
    };

    void add_status_listener(StatusListener& listener);
    void remove_status_listener(StatusListener& listener);


public:
    virtual ~ControllerConnection() = default;

    bool ready() const{ return m_ready.load(std::memory_order_acquire); }
    std::string status_text() const;


protected:
    void signal_ready_changed(bool ready);
    void signal_status_text_changed(const std::string& text);


protected:
    std::atomic<bool> m_ready;

    mutable SpinLock m_status_text_lock;
    std::string m_status_text;

private:
    ListenerSet<StatusListener> m_status_listeners;
};




}
#endif
