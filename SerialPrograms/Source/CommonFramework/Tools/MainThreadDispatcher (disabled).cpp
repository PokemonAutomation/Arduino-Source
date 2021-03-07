/*  Dispatch to Main Thread
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <emmintrin.h>
#include "MainThreadDispatcher (disabled).h"

std::unique_ptr<QApplication> application;

MainThreadDispatcher::MainThreadDispatcher()
    : m_live(0)
{}
MainThreadDispatcher::~MainThreadDispatcher(){
    cout << this << " ~MainThreadDispatcher()" << " : " << m_live << endl;
    while (m_live.load(std::memory_order_acquire) != 0){
        _mm_pause();
    }
}

MainThreadDispatcher::Decrementer::Decrementer(std::atomic<size_t>& live)
    : m_live(live)
{
    cout << &m_live << " MainThreadDispatcher**" << " : " << m_live << endl;
}
MainThreadDispatcher::Decrementer::~Decrementer(){
    m_live.fetch_sub(1);
    cout << &m_live << " MainThreadDispatcher--" << " : " << m_live << endl;
}


