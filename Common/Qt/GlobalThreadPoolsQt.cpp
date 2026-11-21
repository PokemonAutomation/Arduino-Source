/*  Global Thread Pools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GlobalThreadPoolsQt.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{



//  Leaked for the same reason as the non-Qt global pools (see
//  GlobalThreadPools.cpp): the pool destructors join their threads, and a
//  join at process exit can hang without diagnostics. Normal mode stops
//  these explicitly at the end of "main()"; command-line test mode exits
//  without joining.
QtWorkerThreadPool& qt_worker_threadpool(){
    static QtWorkerThreadPool* pool = new QtWorkerThreadPool();
    return *pool;
}


QtEventThreadPool& qt_event_threadpool(){
    static QtEventThreadPool* pool = new QtEventThreadPool();
    return *pool;
}



}
}
