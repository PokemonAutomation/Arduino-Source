/*  QtThreadPool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "QtThreadPool.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



class QtWorkerThread : private QThread{
public:
    QtWorkerThread(QtWorkerThreadPool& parent)
        : m_parent(parent)
    {
        start();
    }
    ~QtWorkerThread(){
//        cout << "~QtWorkerThread() - start" << endl;
        m_cv.notify_all();
        quit();
        QtWorkerThread::wait();
        QThread::wait();
//        cout << "~QtWorkerThread() - end" << endl;
    }

    void run_unprotected(std::function<void()> lambda){
        m_lambda = std::move(lambda);
        m_cv.notify_all();
    }
    void wait(){
        std::unique_lock<Mutex> lg(m_parent.m_lock);
        m_cv.wait(lg, [this]{ return m_lambda == nullptr; });
    }


private:
    virtual void run() override{
//        exec();
        while (true){
            {
                std::unique_lock<Mutex> lg(m_parent.m_lock);
                if (m_parent.m_stopping){
                    break;
                }
                if (m_lambda == nullptr){
                    m_cv.wait(lg);
                    continue;
                }
            }
            m_lambda();
            {
                std::lock_guard<Mutex> lg(m_parent.m_lock);
                m_lambda = nullptr;
                m_parent.m_available_threads.emplace_back(this);
                m_cv.notify_all();
            }
        }
//        cout << "Thread ending..." << endl;
    }

    QtWorkerThreadPool& m_parent;
    std::function<void()> m_lambda;
    ConditionVariable m_cv;
};




QtWorkerThreadPool::QtWorkerThreadPool() = default;
QtWorkerThreadPool::~QtWorkerThreadPool(){
    stop();
}

void QtWorkerThreadPool::stop(){
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_stopping = true;
    }
    m_cv.notify_all();
    m_threads.clear();
}


void QtWorkerThreadPool::dispatch(std::function<void()> lambda){
    if (lambda == nullptr){
        return;
    }
    QtWorkerThread* thread;
    {
        std::unique_lock<Mutex> lg(m_lock);
        if (m_available_threads.empty()){
            m_available_threads.reserve(m_threads.size() + 1);
            auto& new_thread = m_threads.emplace_back(std::make_unique<QtWorkerThread>(*this));
            m_available_threads.emplace_back(new_thread.get());
        }
        thread = m_available_threads.back();
        thread->run_unprotected(std::move(lambda));
    }
}
void QtWorkerThreadPool::run_and_wait(std::function<void()> lambda){
    if (lambda == nullptr){
        return;
    }
    QtWorkerThread* thread;
    {
        std::unique_lock<Mutex> lg(m_lock);
        if (m_available_threads.empty()){
            m_available_threads.reserve(m_threads.size() + 1);
            auto& new_thread = m_threads.emplace_back(std::make_unique<QtWorkerThread>(*this));
            m_available_threads.emplace_back(new_thread.get());
        }
        thread = m_available_threads.back();
        thread->run_unprotected(std::move(lambda));
    }
    thread->wait();
}









QtEventThreadPool::~QtEventThreadPool(){
    stop();
}
void QtEventThreadPool::stop(){
    m_threads.clear();
    m_available_threads.clear();
}






QObject* QtEventThreadPool::add_object(std::function<std::unique_ptr<QObject>()> factory){
    QtEventThread& thread = get_thread();
    QObject* ret = thread.add_object(std::move(factory));
    try{
        m_objects[ret] = &thread;
    }catch (...){
        thread.remove_object();
        std::lock_guard<Mutex> lg(m_lock);
        m_available_threads.emplace_back(&thread);
        throw;
    }
    return ret;
}
void QtEventThreadPool::remove_object(QObject* object) noexcept{
    std::lock_guard<Mutex> lg(m_lock);
    auto iter = m_objects.find(object);
    if (iter == m_objects.end()){
        return;
    }
    iter->second->remove_object();
    m_available_threads.emplace_back(iter->second);
    m_objects.erase(iter);
}

QtEventThread& QtEventThreadPool::get_thread(){
    std::lock_guard<Mutex> lg(m_lock);
    if (m_available_threads.empty()){
        m_available_threads.reserve(m_threads.size() + 1);
        auto& new_thread = m_threads.emplace_back();
        m_available_threads.emplace_back(&new_thread);
    }
    QtEventThread* ret = m_available_threads.back();
    m_available_threads.pop_back();
    return *ret;
}






}
