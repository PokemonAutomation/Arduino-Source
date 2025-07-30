/*  SpinWaitWithEvents
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SpinWaitWithEvents_H
#define PokemonAutomation_SpinWaitWithEvents_H

#include <atomic>
#include <QApplication>
#include "Common/Cpp/Concurrency/SpinPause.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class SpinWaitWithEvents{
public:
    SpinWaitWithEvents()
        : m_finished(false)
    {}
    void process_events_while_waiting() const{
        if (m_finished.load(std::memory_order_acquire)){
            return;
        }
        do{
            QApplication::processEvents();
            pause();
        }while (!m_finished.load(std::memory_order_acquire));
    }
    void signal(){
        m_finished.store(true, std::memory_order_release);
    }

private:
    std::atomic<bool> m_finished;
};



}
#endif
