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
    QtEventThread(){
        start();
    }
    ~QtEventThread(){
        quit();
        wait();
    }

    QObject* add_object(std::function<std::unique_ptr<QObject>()> factory){
        m_pending_factory = std::move(factory);
        emit add_object_internal();
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [this]{ return m_pending_factory == nullptr; });
        return m_object.get();
    }
    void remove_object(){
        emit remove_object_internal();
        std::unique_lock<Mutex> lg(m_lock);
        m_cv.wait(lg, [this]{ return m_object == nullptr; });
    }

public slots:
    void add_object_internal(){
        m_object = m_pending_factory();
    }
    void remove_object_internal(){
        m_object.reset();
    }

private:
    Mutex m_lock;
    ConditionVariable m_cv;

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

    bool m_stopping = false;
    std::deque<QtEventThread> m_threads;
    std::vector<QtEventThread*> m_available_threads;
};




}
#endif
