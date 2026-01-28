/*  QtThreadPool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_QtThreadPool_H
#define PokemonAutomation_QtThreadPool_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <QThread>

namespace PokemonAutomation{


class QWorkerThread;



class QWorkerThreadPool{
public:
    QWorkerThreadPool();
    ~QWorkerThreadPool();
    void stop();

    void dispatch(std::function<void()> lambda);
    void run_and_wait(std::function<void()> lambda);


private:
    friend class QWorkerThread;

    std::vector<std::unique_ptr<QWorkerThread>> m_threads;
    std::vector<QWorkerThread*> m_available_threads;
    bool m_stopping = false;

    std::mutex m_lock;
    std::condition_variable m_cv;
};






}
#endif
