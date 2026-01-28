/*  Global Thread Pools (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "GlobalThreadPoolsQt.h"

namespace PokemonAutomation{
namespace GlobalThreadPools{



QWorkerThreadPool& qt_threadpool(){
    static QWorkerThreadPool pool;
    return pool;
}



}
}
