/*  QtThreadPool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_QtThreadPool_H
#define PokemonAutomation_QtThreadPool_H

#include <vector>
#include <deque>
#include <map>
#include <QThread>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


class QtWorkerThread;
class QtEventThread;


//
//  A simple work-running thread pool.
//  This is functionally the same as Common/Cpp/Concurrency/ThreadPool.h,
//  except that these are Qt threads and it is safe to run signal/slots off them.
//
//  This is deprecated and will be removed in the future.
//
class QtWorkerThreadPool{
public:
    QtWorkerThreadPool();
    ~QtWorkerThreadPool();
    void stop();

    void dispatch(std::function<void()> lambda);
    void run_and_wait(std::function<void()> lambda);


private:
    friend class QtWorkerThread;

    std::vector<std::unique_ptr<QtWorkerThread>> m_threads;
    std::vector<QtWorkerThread*> m_available_threads;
    bool m_stopping = false;

    Mutex m_lock;
    ConditionVariable m_cv;
};





class QtEventThread : public QThread{
    Q_OBJECT

public:
    QtEventThread();
    ~QtEventThread();

    QObject* add_object(std::function<std::unique_ptr<QObject>()> factory);
    void remove_object();

public:
    virtual void run() override;

private:
    void add_object_internal();
    void remove_object_internal();

private:
    Mutex m_lock;
    ConditionVariable m_cv;

    std::atomic<QObject*> m_dummy;

    std::function<std::unique_ptr<QObject>()> m_pending_factory;
    std::unique_ptr<QObject> m_object;
};




//
//  This thread pool serves no purpose than to move objects into dedicated
//  threads so their event loop processing does not conflict with anything
//  else. Stuff like Qt video and audio will do this.
//
class QtEventThreadPool : public QObject{
public:
    ~QtEventThreadPool();
    void stop();

    //  Construct an object on a dedicated thread.
    //  You pass it a factory. It returns a pointer to the object.
    QObject* add_object(std::function<std::unique_ptr<QObject>()> factory);

    //  Remove and destruct the object.
    void remove_object(QObject* object) noexcept;


private:
    QtEventThread& get_thread();


private:
    std::map<QObject*, QtEventThread*> m_objects;

    Mutex m_lock;

    std::vector<std::unique_ptr<QtEventThread>> m_threads;
    std::vector<QtEventThread*> m_available_threads;
};




}
#endif
