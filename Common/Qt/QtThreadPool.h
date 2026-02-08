/*  QtThreadPool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_QtThreadPool_H
#define PokemonAutomation_QtThreadPool_H

#include <vector>
#include <QThread>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"

namespace PokemonAutomation{


class QtWorkerThread;



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






}
#endif
