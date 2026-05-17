/*  Serial Port Poller (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  "QSerialPortInfo::availablePorts()" can take a long time to return.
 *  So we run it in the background as a poller.
 *
 */

#ifndef PokemonAutomation_SerialPortPoller_H
#define PokemonAutomation_SerialPortPoller_H

#include <QSerialPortInfo>
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Concurrency/PeriodicRunner.h"

namespace PokemonAutomation{



class SerialPortPoller : public PeriodicRunner::Runnable{
public:
    static SerialPortPoller& instance();

    void stop();

    QList<QSerialPortInfo> ports() const;

    virtual void run() noexcept override;

private:
    SerialPortPoller();
    ~SerialPortPoller();

    mutable SpinLock m_lock;
    QList<QSerialPortInfo> m_ports;
};





}
#endif
