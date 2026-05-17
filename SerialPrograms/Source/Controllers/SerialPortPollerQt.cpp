/*  Serial Port Poller (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalServices.h"
#include "SerialPortPollerQt.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



SerialPortPoller& SerialPortPoller::instance(){
    static SerialPortPoller poller;
    return poller;
}

SerialPortPoller::SerialPortPoller(){
    global_periodic_runner().add_runnable(*this, std::chrono::seconds(1));
}
SerialPortPoller::~SerialPortPoller(){
    stop();
}
void SerialPortPoller::stop(){
    global_periodic_runner().remove_runnable(*this);
}


QList<QSerialPortInfo> SerialPortPoller::ports() const{
    ReadSpinLock lg(m_lock);
    return m_ports;
}

void SerialPortPoller::run() noexcept{
//    cout << "SerialPortPoller::run()" << endl;
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    WriteSpinLock lg(m_lock);
    m_ports = std::move(ports);
}






}
