/*  Redispatch
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <mutex>
#include <condition_variable>
#include <QThread>
#include <QApplication>
#include "Redispatch.h"

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

    std::mutex lock;
    std::condition_variable cv;
    bool done = false;
    QMetaObject::invokeMethod(object, [&]{
        lambda();
        std::lock_guard<std::mutex> lg(lock);
        done = true;
        cv.notify_all();
    });
    std::unique_lock<std::mutex> lg(lock);
    cv.wait(lg, [&]{ return done; });
}




}
