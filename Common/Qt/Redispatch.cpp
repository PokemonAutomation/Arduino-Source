/*  Redispatch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QThread>
#include <QApplication>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Redispatch.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


void dispatch_to_main_thread(std::function<void()> lambda){
    QMetaObject::invokeMethod(QCoreApplication::instance(), lambda);
}
void queue_on_main_thread(std::function<void()> lambda){
    QMetaObject::invokeMethod(QCoreApplication::instance(), lambda, Qt::QueuedConnection);
}


void run_on_main_thread_and_wait(std::function<void()> lambda){
    run_on_object_thread_and_wait(QCoreApplication::instance(), std::move(lambda));
}

void run_on_object_thread_and_wait(QObject* object, std::function<void()> lambda){
    if (object->thread() == QThread::currentThread()){
        lambda();
        return;
    }

//    static int c = 0;
//    c++;

    Mutex lock;
    ConditionVariable cv;
    bool done = false;
//    cout << c << ": " << "Invoking..." << endl;
    QMetaObject::invokeMethod(object, [&]{
//        cout << c << ": " << "Running Lambda..." << endl;
        lambda();
//        cout << c << ": " << "Running Lambda... Done" << endl;
        std::lock_guard<Mutex> lg(lock);
        done = true;
        cv.notify_all();
    });
//    cout << c << ": " << "Invoking... Done" << endl;
    std::unique_lock<Mutex> lg(lock);
//    cout << c << ": " << "Waiting..." << endl;
    cv.wait(lg, [&]{ return done; });
}




}
