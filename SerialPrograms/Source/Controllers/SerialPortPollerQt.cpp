/*  Serial Port Poller (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
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
    , m_highest(WallDuration(0))
{
    global_periodic_runner().add_runnable(*this, std::chrono::seconds(1));
}
SerialPortPoller::~SerialPortPoller(){
    stop();
}
void SerialPortPoller::stop(){
    global_periodic_runner().remove_runnable(*this);
}


void SerialPortPoller::begin_refresh_now(){
    global_periodic_runner().trigger_run_now(*this);
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
//    cout << "SerialPortPoller::run() - start" << endl;
//    global_logger_tagged().log("Refreshing serial ports...");
//    cout << "SerialPortPoller::run() - start" << endl;

    try{
        WallClock start = current_time();

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

        WallClock end = current_time();

        //  Throttle the next call.
        WallDuration duration = end - start;
        WallDuration next_delay = duration * 10;
        if (next_delay < std::chrono::seconds(1)){
            next_delay = std::chrono::seconds(1);
        }
        if (GlobalSettings::instance().LOG_EVERYTHING){
            global_logger_tagged().log(
                "Serial port refresh took " + tostr_u_commas(std::chrono::duration_cast<Milliseconds>(duration).count()) + " ms.",
                COLOR_ORANGE
            );
        }else if (m_highest < duration){
            m_highest = duration;
            global_logger_tagged().log(
                "Serial port refresh took " + tostr_u_commas(std::chrono::duration_cast<Milliseconds>(duration).count()) + " ms. "
                "Further updates shorter than this will be suppressed.",
                COLOR_ORANGE
            );
        }
        global_periodic_runner().edit_duration(*this, next_delay);

    }catch (...){}
//    cout << "SerialPortPoller::run() - end" << endl;
}






}
