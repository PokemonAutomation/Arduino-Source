/*  Serial Port Poller (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
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
    try{
        QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
        std::set<std::string> current;
        for (auto& port : ports){
            current.insert(port.portName().toStdString());
        }

        WriteSpinLock lg(m_lock);
        auto iter0 = m_last.begin();
        auto iter1 = current.begin();
        while (iter0 != m_last.end() && iter1 != current.end()){
            if (*iter0 < *iter1){
                global_logger_tagged().log("Serial Port Removed: " + *iter0, COLOR_RED);
                ++iter0;
            }else if (*iter0 > *iter1){
                global_logger_tagged().log("Serial Port Added: " + *iter1, COLOR_BLUE);
                ++iter1;
            }else{
                ++iter0;
                ++iter1;
            }
        }
        for (; iter0 != m_last.end(); ++iter0){
            global_logger_tagged().log("Serial Port Removed: " + *iter0, COLOR_RED);
        }
        for (; iter1 != current.end(); ++iter1){
            global_logger_tagged().log("Serial Port Added: " + *iter1, COLOR_BLUE);
        }
        m_last = std::move(current);
        m_ports = std::move(ports);
    }catch (...){}
}






}
