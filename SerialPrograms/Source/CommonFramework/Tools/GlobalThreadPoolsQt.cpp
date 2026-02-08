/*  Global Thread Pools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GlobalThreadPoolsQt.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{



QtWorkerThreadPool& qt_worker_threadpool(){
    static QtWorkerThreadPool pool;
    return pool;
}



}
}
