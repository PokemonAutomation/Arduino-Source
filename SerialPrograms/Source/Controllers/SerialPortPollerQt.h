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
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/PeriodicRunner.h"

namespace PokemonAutomation{



class SerialPortPoller : public PeriodicRunner::Runnable{
public:
    static SerialPortPoller& instance();

    void stop();

    void begin_refresh_now();
    QList<QSerialPortInfo> ports() const;
    QSerialPortInfo get_port(const std::string& name) const;

    virtual void run() noexcept override;

private:
    SerialPortPoller();
    ~SerialPortPoller();

    mutable SpinLock m_lock;
    std::map<std::string, QSerialPortInfo> m_last;
    QList<QSerialPortInfo> m_list;
};





}
#endif
