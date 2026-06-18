/*  Serial Port Poller (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  "QSerialPortInfo::availablePorts()" can take a long time to return.
 *  This can hang the UI which is bad. So we run it in the background
 *  as a poller.
 *
 */

#ifndef PokemonAutomation_SerialPortPoller_H
#define PokemonAutomation_SerialPortPoller_H

#include <map>
#include <QSerialPortInfo>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/PeriodicRunner.h"

namespace PokemonAutomation{



class SerialPortPoller : public PeriodicRunner::Runnable{
public:
    struct Listener{
        virtual void on_serial_ports_changed(const QList<QSerialPortInfo>& ports) = 0;
    };
    void add_listener(Listener& listener){
        m_listeners.add(listener);
    }
    void remove_listener(Listener& listener){
        m_listeners.remove(listener);
    }

public:
    static SerialPortPoller& instance();

    void stop();

    void begin_refresh_now();
    WallClock last_changed() const;
    QList<QSerialPortInfo> ports() const;
    QSerialPortInfo get_port(const std::string& name) const;

    virtual void run() noexcept override;

private:
    SerialPortPoller();
    ~SerialPortPoller();

    mutable SpinLock m_lock;
    WallClock m_last_change;
    std::map<std::string, QSerialPortInfo> m_last;
    QList<QSerialPortInfo> m_list;

    ListenerSet<Listener> m_listeners;
};





}
#endif
