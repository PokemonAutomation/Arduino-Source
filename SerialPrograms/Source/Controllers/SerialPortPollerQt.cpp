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

SerialPortPoller::SerialPortPoller()
    : m_last_change(current_time())
{
    global_periodic_runner().add_runnable(*this, std::chrono::seconds(10));
}
SerialPortPoller::~SerialPortPoller(){
    stop();
}
void SerialPortPoller::stop(){
    global_periodic_runner().remove_runnable(*this);
}


void SerialPortPoller::begin_refresh_now(){
    global_periodic_runner().run_now_nonblocking(*this);
}
WallClock SerialPortPoller::last_changed() const{
    ReadSpinLock lg(m_lock);
    return m_last_change;
}
QList<QSerialPortInfo> SerialPortPoller::ports() const{
    ReadSpinLock lg(m_lock);
    return m_list;
}

QSerialPortInfo SerialPortPoller::get_port(const std::string& name) const{
    ReadSpinLock lg(m_lock);
    auto iter = m_last.find(name);
    if (iter == m_last.end()){
        return QSerialPortInfo();
    }
    return iter->second;
}

void SerialPortPoller::run() noexcept{
//    global_logger_tagged().log("Refreshing serial ports...");
//    cout << "SerialPortPoller::run() - start" << endl;
    try{
        QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
        std::map<std::string, QSerialPortInfo> current;
        for (auto& port : list){
            current.emplace(port.portName().toStdString(), port);
        }

        WallClock now = current_time();
        bool changed = false;

        WriteSpinLock lg(m_lock);
        auto iter0 = m_last.begin();
        auto iter1 = current.begin();
        while (iter0 != m_last.end() && iter1 != current.end()){
            if (iter0->first < iter1->first){
                global_logger_tagged().log("Serial Port Removed: " + iter0->first, COLOR_RED);
                changed = true;
                ++iter0;
            }else if (iter0->first > iter1->first){
                global_logger_tagged().log("Serial Port Added: " + iter1->first, COLOR_BLUE);
                changed = true;
                ++iter1;
            }else{
                ++iter0;
                ++iter1;
            }
        }
        for (; iter0 != m_last.end(); ++iter0){
            global_logger_tagged().log("Serial Port Removed: " + iter0->first, COLOR_RED);
            changed = true;
        }
        for (; iter1 != current.end(); ++iter1){
            global_logger_tagged().log("Serial Port Added: " + iter1->first, COLOR_BLUE);
            changed = true;
        }
        m_last = std::move(current);
        m_list = std::move(list);

        if (changed){
            m_last_change = now;
            m_listeners.run_method(&Listener::on_serial_ports_changed, m_list);
        }

    }catch (...){}
//    cout << "SerialPortPoller::run() - end" << endl;
}






}
