/*  Computation Thread Pool
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "AsyncTask.h"
#include "ComputationThreadPoolCore.h"
#include "ComputationThreadPool.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{









ComputationThreadPool::ComputationThreadPool(
    std::function<void()>&& new_thread_callback,
    size_t starting_threads,
    size_t max_threads
)
    : m_core(
        CONSTRUCT_TOKEN,
        std::move(new_thread_callback),
        starting_threads,
        max_threads
    )
{}
ComputationThreadPool::~ComputationThreadPool() = default;
size_t ComputationThreadPool::current_threads() const{
    return m_core->current_threads();
}
size_t ComputationThreadPool::max_threads() const{
    return m_core->max_threads();
}
WallDuration ComputationThreadPool::cpu_time() const{
    return m_core->cpu_time();
}
void ComputationThreadPool::ensure_threads(size_t threads){
    m_core->ensure_threads(threads);
}
//void ParallelTaskRunner::wait_for_everything(){
//    m_core->wait_for_everything();
//}
std::unique_ptr<AsyncTask> ComputationThreadPool::blocking_dispatch(std::function<void()>&& func){
    return m_core->blocking_dispatch(std::move(func));
}
std::unique_ptr<AsyncTask> ComputationThreadPool::try_dispatch(std::function<void()>& func){
    return m_core->try_dispatch(func);
}
void ComputationThreadPool::run_in_parallel(
    const std::function<void(size_t index)>& func,
    size_t start, size_t end,
    size_t block_size
){
    m_core->run_in_parallel(func, start, end, block_size);
}
















}
