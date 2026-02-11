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



class QtEventThreadPool : public QObject{
public:
    ~QtEventThreadPool();
    void stop();

    QObject* add_object(std::function<std::unique_ptr<QObject>()> factory);
    void remove_object(QObject* object) noexcept;


private:
    QtEventThread& get_thread();


private:
    std::map<QObject*, QtEventThread*> m_objects;

    Mutex m_lock;

    std::deque<QtEventThread> m_threads;
    std::vector<QtEventThread*> m_available_threads;
};




}
#endif
