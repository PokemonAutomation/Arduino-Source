/*  Dispatch to Main Thread
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#ifndef PokemonAutomation_MainThreadDispatcher_H
#define PokemonAutomation_MainThreadDispatcher_H

#include <atomic>
#include <memory>
#include <QApplication>

#include <iostream>
using std::cout;
using std::endl;


extern std::unique_ptr<QApplication> application;


class MainThreadDispatcher{
public:
    MainThreadDispatcher();
    ~MainThreadDispatcher();

    template <typename Lambda>
    void operator()(Lambda&& lambda){
        m_live.fetch_add(1);
        cout << this << " MainThreadDispatcher++" << " : " << m_live << endl;

        try{
            bool ok = QMetaObject::invokeMethod(
                application.get(),
                [=, lambda = std::move(lambda)]{
                    Decrementer decrementer(m_live);
                    lambda();
                }
            );
            if (!ok){
                cout << this << " Dispatch Failed" << " : " << m_live << endl;
                m_live.fetch_sub(1);
            }
        }catch (...){
            cout << this << " Dispatch Exception" << " : " << m_live << endl;
            m_live.fetch_sub(1);
            throw;
        }
    }

private:
    struct Decrementer{
        Decrementer(std::atomic<size_t>& live);
        ~Decrementer();
        std::atomic<size_t>& m_live;
    };

private:
    std::atomic<size_t> m_live;
};


#endif
