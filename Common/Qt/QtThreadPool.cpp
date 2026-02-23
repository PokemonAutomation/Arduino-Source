/*  QtThreadPool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Concurrency/Qt6.9ThreadBugWorkaround.h"
#include "Common/Cpp/Concurrency/SpinPause.h"
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
#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND
    //  Leak all the threads because they can't be joined without hanging.
    for (std::unique_ptr<QtWorkerThread>& thread : m_threads){
        thread.release();
    }
#endif
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








QtEventThread::QtEventThread(){
    start();

    //  Wait for the thread to fully start up and construct the body.
    while (m_dummy.load(std::memory_order_acquire) == nullptr){
        pause();
    }
}
QtEventThread::~QtEventThread(){
    m_dummy.store(nullptr, std::memory_order_relaxed);
    quit();
    wait();
}
QObject* QtEventThread::add_object(std::function<std::unique_ptr<QObject>()> factory){
    m_pending_factory = std::move(factory);
    QMetaObject::invokeMethod(m_dummy.load(std::memory_order_relaxed), [this]{
        add_object_internal();
    });
    std::unique_lock<Mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_object != nullptr; });
    return m_object.get();
}
void QtEventThread::remove_object(){
    QMetaObject::invokeMethod(m_dummy.load(std::memory_order_relaxed), [this]{
        remove_object_internal();
    });
    std::unique_lock<Mutex> lg(m_lock);
    m_cv.wait(lg, [this]{ return m_object == nullptr; });
}
void QtEventThread::run(){
    setPriority(QThread::TimeCriticalPriority);

    QObject dummy;
    m_dummy.store(&dummy, std::memory_order_release);
    exec();

    //  Wait until we are in the destructor before destroying the body.
    while (m_dummy.load(std::memory_order_acquire) != nullptr){
        pause();
    }
}
void QtEventThread::add_object_internal(){
    {
        std::lock_guard<Mutex> lg(m_lock);
//        cout << "constructing on: " << std::this_thread::get_id() << endl;
        m_object = m_pending_factory();
        m_pending_factory = nullptr;
    }
    m_cv.notify_all();
}
void QtEventThread::remove_object_internal(){
    {
        std::lock_guard<Mutex> lg(m_lock);
//        cout << "destructing on: " << std::this_thread::get_id() << endl;
        m_object.reset();
    }
    m_cv.notify_all();
}






QtEventThreadPool::~QtEventThreadPool(){
    stop();
}
void QtEventThreadPool::stop(){
#ifdef PA_ENABLE_QT_ADOPTION_WORKAROUND
    //  Leak all the threads because they can't be joined without hanging.
    for (std::unique_ptr<QtEventThread>& thread : m_threads){
        thread.release();
    }
#endif
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
    std::map<QObject*, QtEventThread*>::iterator iter;
    {
        std::lock_guard<Mutex> lg(m_lock);
        iter = m_objects.find(object);
        if (iter == m_objects.end()){
            return;
        }
    }
    iter->second->remove_object();
    {
        std::lock_guard<Mutex> lg(m_lock);
        m_available_threads.emplace_back(iter->second);
        m_objects.erase(iter);
    }
}

QtEventThread& QtEventThreadPool::get_thread(){
    std::lock_guard<Mutex> lg(m_lock);
    if (m_available_threads.empty()){
        m_available_threads.reserve(m_threads.size() + 1);
        QtEventThread* new_thread = m_threads.emplace_back(
            std::make_unique<QtEventThread>()
        ).get();
        m_available_threads.emplace_back(new_thread);
    }
    QtEventThread* ret = m_available_threads.back();
//    cout << "QtEventThreadPool: " << m_threads.size() << ", using = " << ret << endl;
    m_available_threads.pop_back();
    return *ret;
}






}
